import pandas as pd
import matplotlib.pyplot as plt

filename = "build/results.csv"

# Читаем результаты
data = pd.read_csv(filename)

# Преобразуем таблицу в формат для heatmap
heatmap_data = data.pivot(
    index="cache",
    columns="pattern",
    values="hit_rate"
)

plt.figure(figsize=(10, 5))

plt.imshow(
    heatmap_data,
    aspect="auto"
)

# Подписи осей
plt.xticks(
    range(len(heatmap_data.columns)),
    heatmap_data.columns
)

plt.yticks(
    range(len(heatmap_data.index)),
    heatmap_data.index
)

plt.xlabel("Тип нагрузки")
plt.ylabel("Алгоритм кэширования")
plt.title("Hit rate алгоритмов кэширования")

# Показываем значения внутри ячеек
for i in range(len(heatmap_data.index)):
    for j in range(len(heatmap_data.columns)):
        value = heatmap_data.iloc[i, j]

        plt.text(
            j,
            i,
            f"{value:.2f}%",
            ha="center",
            va="center"
        )

plt.colorbar(label="Hit rate (%)")

plt.tight_layout()
plt.show()