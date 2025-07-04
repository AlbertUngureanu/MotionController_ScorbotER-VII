import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import numpy as np

# Simulează date pentru 5 mișcări
def genereaza_date():
    t = np.linspace(0, 10, 200)
    curenti = [np.sin(t + i) + np.random.normal(0, 0.1, t.shape) for i in range(5)]
    return t, curenti

# Creează fereastra separată și afișează graficele
def afiseaza_grafice():
    t, curenti = genereaza_date()

    # Fereastra nouă
    top = tk.Toplevel(root)
    top.title("Grafice curent - Robot Industrial")

    grafice_frame = tk.Frame(top)
    grafice_frame.pack(padx=10, pady=10)

    for i in range(5):
        frame = tk.Frame(grafice_frame)
        frame.grid(row=i // 2, column=i % 2, padx=5, pady=5)

        fig, ax = plt.subplots(figsize=(4, 2))
        ax.plot(t, curenti[i])
        ax.set_title(f"Mișcare {i+1}")
        ax.set_xlabel("Timp (s)")
        ax.set_ylabel("Curent (A)")
        fig.tight_layout()

        canvas = FigureCanvasTkAgg(fig, master=frame)
        canvas.draw()
        canvas.get_tk_widget().pack()

# UI principală
root = tk.Tk()
root.title("Control Robot")

btn = tk.Button(root, text="Afișează graficele", command=afiseaza_grafice)
btn.pack(pady=20)

root.mainloop()
