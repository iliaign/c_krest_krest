import sys
import os
import pandas as pd
import matplotlib.pyplot as plt

filename = "time_results.csv"
filepath = os.path.join("build", filename)

# Читаем данные из CSV
data = pd.read_csv(filepath)

requests = data["requests"]

# Строим график
plt.figure(figsize=(10, 6))

plt.plot(requests, data["LFU"], marker="o", label="LFU")
plt.plot(requests, data["2Q"], marker="o", label="2Q")
plt.plot(requests, data["ARC"], marker="o", label="ARC")
plt.plot(requests, data["LIRS"], marker="o", label="LIRS")

plt.xscale("log")
plt.yscale("log")

plt.xlabel("Количество запросов")
plt.ylabel("Время выполнения (мкс)")
plt.title("Зависимость времени выполнения от количества запросов")

plt.grid(True)
plt.legend()
plt.tight_layout()

plt.show()