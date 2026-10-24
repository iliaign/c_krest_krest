from pathlib import Path
import math
import sys

try:
    import pandas as pd
    import matplotlib.pyplot as plt
except ImportError as exc:
    print(f"Не найдена библиотека: {exc.name}")
    print("Установи зависимости: python -m pip install pandas matplotlib")
    raise SystemExit(1)

BASE = Path(__file__).resolve().parent
OUT = BASE / "analysis_output"
FIG = OUT / "figures"
OUT.mkdir(exist_ok=True)
FIG.mkdir(exist_ok=True)

SINGLE_FILE = BASE / "build/results.csv"
TWO_FILE = BASE / "build/two_level_results.csv"
TIME_FILE = BASE / "build/time_results.csv"

# В этом проекте hit_rate записывается в процентах: 0..100.
# belady_ratio = hit_rate алгоритма / hit_rate Belady.
# Поэтому ratio=1.0 означает совпадение с Belady, а 0.9 — 90% от его hit rate.


def load_csv(path):
    if not path.exists():
        print(f"[НЕТ ФАЙЛА] {path.name}")
        return None
    try:
        df = pd.read_csv(path)
    except Exception as exc:
        print(f"[ОШИБКА] Не удалось прочитать {path.name}: {exc}")
        return None
    if df.empty:
        print(f"[ПУСТО] {path.name}")
        return None
    print(f"[OK] {path.name}: {len(df)} строк")
    return df


def numeric(df, cols):
    for col in cols:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")
    return df


def save_plot(fig, filename):
    fig.tight_layout()
    fig.savefig(FIG / filename, dpi=180, bbox_inches="tight")
    plt.close(fig)


def summarize_single(df):
    required = {"pattern", "capacity", "seed", "cache", "hit_rate", "belady_ratio"}
    if not required.issubset(df.columns):
        print("[ПРОПУСК] results.csv: не хватает столбцов:",
              ", ".join(sorted(required - set(df.columns))))
        return None

    d = numeric(df.copy(), ["capacity", "seed", "requests", "hits", "misses",
                            "hit_rate", "belady_ratio"])
    d = d.dropna(subset=["hit_rate", "belady_ratio", "capacity"])
    # Belady сам сравнивается с собой и всегда имеет ratio=1; исключаем его
    # из рейтинга конкурирующих алгоритмов, но используем для hit rate-эталона.
    belady = d[d["cache"].astype(str).str.lower() == "belady"]
    algos = d[d["cache"].astype(str).str.lower() != "belady"].copy()

    # Сопоставляем эталон по тому же паттерну, ёмкости и seed.
    keys = [c for c in ["category", "pattern", "capacity", "seed", "requests"]
            if c in d.columns and c in belady.columns]
    if not keys:
        print("[ПРОПУСК] Не удалось сопоставить результаты с Belady.")
        return None

    ref = belady[keys + ["hit_rate"]].rename(columns={"hit_rate": "belady_hit_rate"})
    # На случай дубликатов не размножаем строки при соединении.
    ref = ref.drop_duplicates(subset=keys)
    algos = algos.merge(ref, on=keys, how="left")
    algos = algos.dropna(subset=["belady_hit_rate"])
    algos["gap_pp"] = algos["belady_hit_rate"] - algos["hit_rate"]
    # Сколько процентов от hit rate Belady набирает алгоритм.
    # belady_ratio уже записан C++ кодом, но дополнительно считаем проверочную метрику.
    algos["ratio_pct"] = algos["belady_ratio"] * 100
    algos["gap_pct_of_belady"] = 100 - algos["ratio_pct"]

    group = ["pattern", "capacity", "cache"]
    if "category" in algos.columns:
        group.insert(0, "category")
    summary = (
        algos.groupby(group, dropna=False)
        .agg(
            hit_rate_mean=("hit_rate", "mean"),
            hit_rate_std=("hit_rate", "std"),
            belady_hit_rate_mean=("belady_hit_rate", "mean"),
            ratio_pct_mean=("ratio_pct", "mean"),
            ratio_pct_std=("ratio_pct", "std"),
            gap_pp_mean=("gap_pp", "mean"),
            gap_pp_std=("gap_pp", "std"),
            gap_pct_of_belady_mean=("gap_pct_of_belady", "mean"),
            runs=("belady_ratio", "count"),
        )
        .reset_index()
    )
    summary.to_csv(OUT / "single_by_pattern.csv", index=False)

    cap_group = ["capacity", "cache"]
    by_cap = (
        algos.groupby(cap_group, dropna=False)
        .agg(
            ratio_pct_mean=("ratio_pct", "mean"),
            ratio_pct_std=("ratio_pct", "std"),
            gap_pp_mean=("gap_pp", "mean"),
            hit_rate_mean=("hit_rate", "mean"),
            runs=("belady_ratio", "count"),
        ).reset_index()
    )
    by_cap.to_csv(OUT / "single_by_capacity.csv", index=False)

    # График 1: относительная близость к Belady, отдельная линия для каждого алгоритма.
    for pattern, part in summary.groupby("pattern"):
        fig, ax = plt.subplots(figsize=(8.5, 5))
        for cache, a in part.groupby("cache"):
            a = a.sort_values("capacity")
            ax.plot(a["capacity"], a["ratio_pct_mean"], marker="o", label=str(cache))
        ax.axhline(100, linestyle="--", linewidth=1, label="Belady (100%)")
        ax.set_title(f"Близость к Belady: {pattern}")
        ax.set_xlabel("Ёмкость кэша")
        ax.set_ylabel("Hit rate относительно Belady, %")
        ax.grid(True, alpha=0.25)
        ax.legend()
        safe = "".join(ch if ch.isalnum() or ch in "-_" else "_" for ch in str(pattern))
        save_plot(fig, f"belady_ratio_{safe}.png")

    # График 2: тепловая карта разрыва по шаблонам и алгоритмам для каждой ёмкости.
    for capacity, part in summary.groupby("capacity"):
        pivot = part.pivot(index="pattern", columns="cache", values="gap_pct_of_belady_mean")
        fig, ax = plt.subplots(figsize=(10, max(4.5, 0.45 * len(pivot))))
        im = ax.imshow(pivot.values, aspect="auto", interpolation="nearest")
        ax.set_xticks(range(len(pivot.columns)), labels=[str(x) for x in pivot.columns])
        ax.set_yticks(range(len(pivot.index)), labels=[str(x) for x in pivot.index])
        ax.set_title(f"Отставание от Belady при ёмкости {capacity}")
        ax.set_xlabel("Алгоритм")
        ax.set_ylabel("Шаблон запросов")
        fig.colorbar(im, ax=ax, label="Отставание от Belady, %")
        save_plot(fig, f"heatmap_gap_capacity_{capacity}.png")

    # Общий рейтинг — усредняем по шаблонам, ёмкостям и seed.
    overall = (
        algos.groupby("cache")
        .agg(
            ratio_pct_mean=("ratio_pct", "mean"),
            ratio_pct_median=("ratio_pct", "median"),
            gap_pp_mean=("gap_pp", "mean"),
            gap_pct_of_belady_mean=("gap_pct_of_belady", "mean"),
            hit_rate_mean=("hit_rate", "mean"),
            runs=("belady_ratio", "count"),
        ).reset_index()
        .sort_values("ratio_pct_mean", ascending=False)
    )
    overall.to_csv(OUT / "overall_algorithm_ranking.csv", index=False)

    fig, ax = plt.subplots(figsize=(8.5, 4.8))
    p = overall.sort_values("ratio_pct_mean")
    ax.barh(p["cache"].astype(str), p["ratio_pct_mean"])
    ax.axvline(100, linestyle="--", linewidth=1, label="Belady (100%)")
    ax.set_xlabel("Средний hit rate относительно Belady, %")
    ax.set_title("Общее сравнение алгоритмов с Belady")
    ax.grid(axis="x", alpha=0.25)
    ax.legend()
    save_plot(fig, "overall_belady_comparison.png")

    return algos, summary, overall


def summarize_two_level(df):
    required = {"pattern", "total_capacity", "seed", "kind", "hit_rate", "belady_ratio"}
    if not required.issubset(df.columns):
        print("[ПРОПУСК] two_level_results.csv: не хватает столбцов:",
              ", ".join(sorted(required - set(df.columns))))
        return None

    d = numeric(df.copy(), ["total_capacity", "seed", "hit_rate", "belady_ratio",
                            "l1_capacity", "l2_capacity", "requests"])
    d = d.dropna(subset=["hit_rate", "belady_ratio", "total_capacity"])
    d = d[d["kind"].astype(str).str.lower() == "two_level"].copy()
    if d.empty:
        print("[ИНФО] Нет строк kind=two_level.")
        return None

    d["ratio_pct"] = d["belady_ratio"] * 100
    d["gap_pct_of_belady"] = 100 - d["ratio_pct"]
    group = ["pattern", "total_capacity", "mode", "l1_algo", "l2_algo",
             "l1_capacity", "l2_capacity"]
    group = [c for c in group if c in d.columns]
    summary = (
        d.groupby(group, dropna=False)
        .agg(
            hit_rate_mean=("hit_rate", "mean"),
            ratio_pct_mean=("ratio_pct", "mean"),
            ratio_pct_std=("ratio_pct", "std"),
            gap_pct_of_belady_mean=("gap_pct_of_belady", "mean"),
            runs=("belady_ratio", "count"),
        ).reset_index()
        .sort_values("ratio_pct_mean", ascending=False)
    )
    summary.to_csv(OUT / "two_level_by_pattern.csv", index=False)

    # Лучшие двухуровневые конфигурации по каждому шаблону и общей ёмкости.
    idx = summary.groupby(["pattern", "total_capacity"])["ratio_pct_mean"].idxmax()
    summary.loc[idx].sort_values(["pattern", "total_capacity"]).to_csv(
        OUT / "two_level_best_by_pattern.csv", index=False
    )

    # График топ-5 конфигураций в каждом паттерне — чтобы не получить нечитаемый график.
    for pattern, part in summary.groupby("pattern"):
        top = part.nlargest(5, "ratio_pct_mean").copy()
        if top.empty:
            continue
        labels = (
            top.get("l1_algo", "").astype(str) + "/" +
            top.get("l2_algo", "").astype(str) + " | " +
            top.get("mode", "").astype(str) + " | " +
            top.get("l1_capacity", "").astype(str) + "/" +
            top.get("l2_capacity", "").astype(str)
        )
        fig, ax = plt.subplots(figsize=(10, 5))
        ax.barh(labels.iloc[::-1], top["ratio_pct_mean"].iloc[::-1])
        ax.axvline(100, linestyle="--", linewidth=1, label="Belady (100%)")
        ax.set_xlabel("Hit rate относительно Belady, %")
        ax.set_title(f"Лучшие двухуровневые конфигурации: {pattern}")
        ax.grid(axis="x", alpha=0.25)
        ax.legend()
        safe = "".join(ch if ch.isalnum() or ch in "-_" else "_" for ch in str(pattern))
        save_plot(fig, f"two_level_top5_{safe}.png")
    return summary


def write_report(single_result, two_summary, timing):
    lines = [
        "# Отчёт по экспериментам NHWC",
        "",
        "## Как читать основную метрику",
        "",
        "`belady_ratio` рассчитывается как `hit_rate алгоритма / hit_rate Belady`.",
        "В отчёте он представлен в процентах: 100% означает равный hit rate с Belady;",
        "значение ниже 100% показывает относительное отставание.",
        "`gap_pp` — разница hit rate в процентных пунктах: `hit_rate Belady - hit_rate алгоритма`.",
        "Это разные показатели: первый относительный, второй абсолютный.",
        "",
        "Belady используется как офлайн-ориентир только при одинаковой последовательности запросов",
        "и одинаковой ёмкости. Значение ratio не следует трактовать как доказательство оптимальности",
        "реализаций на любых других последовательностях.",
        "",
    ]
    if single_result:
        algos, summary, overall = single_result
        lines += ["## Общий рейтинг относительно Belady", "",
                  "| Алгоритм | Средний hit rate относительно Belady | Среднее отставание, % | Запусков |",
                  "|---|---:|---:|---:|"]
        for _, r in overall.iterrows():
            lines.append(
                f"| {r['cache']} | {r['ratio_pct_mean']:.2f}% | "
                f"{r['gap_pct_of_belady_mean']:.2f}% | {int(r['runs'])} |"
            )
        lines += ["", "## Лучший алгоритм для каждого шаблона и ёмкости", "",
                  "| Шаблон | Ёмкость | Алгоритм | Относительно Belady | Разрыв, п.п. |",
                  "|---|---:|---|---:|---:|"]
        for (pattern, cap), part in summary.groupby(["pattern", "capacity"]):
            best = part.sort_values("ratio_pct_mean", ascending=False).iloc[0]
            lines.append(
                f"| {pattern} | {cap:g} | {best['cache']} | "
                f"{best['ratio_pct_mean']:.2f}% | {best['gap_pp_mean']:.3f} |"
            )
        lines += ["", "## Основные графики", "",
                  "### Общее сравнение", "",
                  "![Общее сравнение с Belady](figures/overall_belady_comparison.png)", "",
                  "Графики по каждому шаблону и тепловые карты находятся в каталоге `figures/`.", ""]
    else:
        lines += ["Одиночные результаты не удалось обработать: проверь `results.csv`.", ""]

    if two_summary is not None and not two_summary.empty:
        best = pd.read_csv(OUT / "two_level_best_by_pattern.csv")
        lines += ["## Двухуровневый кэш", "",
                  "Таблица `two_level_by_pattern.csv` содержит сводку конфигураций,",
                  "а `two_level_best_by_pattern.csv` — лучшую конфигурацию для каждого шаблона и общей ёмкости.",
                  "Лучшие конфигурации выбираются по среднему `belady_ratio`.", ""]
        lines += ["| Шаблон | Общая ёмкость | L1/L2 алгоритмы | Режим | Относительно Belady |",
                  "|---|---:|---|---|---:|"]
        for _, r in best.iterrows():
            lines.append(
                f"| {r.get('pattern', '')} | {r.get('total_capacity', '')} | "
                f"{r.get('l1_algo', '')}/{r.get('l2_algo', '')} | {r.get('mode', '')} | "
                f"{r.get('ratio_pct_mean', float('nan')):.2f}% |"
            )
        lines.append("")

    if timing is not None:
        lines += ["## Время выполнения", "",
                  "Тайминги приведены как дополнительная характеристика и не используются для",
                  "определения близости к Belady. Подробные замеры находятся в `time_results.csv`.", ""]

    lines += [
        "## Ограничения интерпретации",
        "",
        "- Средние значения объединяют разные шаблоны и ёмкости; для выводов по конкретной нагрузке смотрите отдельные таблицы и графики.",
        "- Для случайных шаблонов стандартное отклонение отражает вариативность между seed.",
        "- Если hit rate Belady равен нулю, C++-функция записывает `belady_ratio = 1.0`; такие строки требуют отдельной осторожности при интерпретации.",
        "- Belady — офлайн-эталон: он знает будущую последовательность запросов, в отличие от онлайн-алгоритмов.",
        "",
    ]
    (OUT / "report.md").write_text("\n".join(lines), encoding="utf-8")


def main():
    print("=== Анализ NHWC: фокус на Belady ===")
    single = load_csv(SINGLE_FILE)
    two = load_csv(TWO_FILE)
    timing = load_csv(TIME_FILE)

    single_result = summarize_single(single) if single is not None else None
    two_summary = summarize_two_level(two) if two is not None else None
    write_report(single_result, two_summary, timing)

    print(f"\nГотово. Таблицы, графики и отчёт сохранены в: {OUT}")
    print(f"Отчёт: {OUT / 'report.md'}")
    if single_result is None and two_summary is None:
        print("Пока нет подходящих CSV с результатами. Сначала запусти C++ benchmark.")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
