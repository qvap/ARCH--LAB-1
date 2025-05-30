import customtkinter as ctk
import socket
import struct
import uuid
import re

ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")

def ip_to_int(ip):
    return struct.unpack("!I", socket.inet_aton(ip))[0]

def int_to_ip(ip_int):
    return socket.inet_ntoa(struct.pack("!I", ip_int))

def calculate_netmask(ip1, ip2):
    diff = ip1 ^ ip2
    mask = 0xFFFFFFFF
    while diff:
        mask <<= 1
        diff >>= 1
    return mask & 0xFFFFFFFF

def get_mac_address():
    mac = uuid.getnode()
    return ':'.join(f'{(mac >> ele) & 0xff:02x}' for ele in range(40, -1, -8))

def analyze_ip_range():
    input_text = entry.get().strip()
    match = re.match(r'(\d+\.\d+\.\d+\.\d+)\s*-\s*(\d+\.\d+\.\d+\.\d+)', input_text)
    if not match:
        output_label.configure(text="Неверный формат! Используйте: IP1 - IP2")
        return

    ip1 = ip_to_int(match.group(1))
    ip2 = ip_to_int(match.group(2))
    if ip1 > ip2:
        ip1, ip2 = ip2, ip1

    netmask = calculate_netmask(ip1, ip2)
    network = ip1 & netmask
    broadcast = network | ~netmask & 0xFFFFFFFF

    output = (
        f"Начальный IP:     {int_to_ip(ip1)}\n"
        f"Конечный IP:      {int_to_ip(ip2)}\n"
        f"Адрес сети:       {int_to_ip(network)}\n"
        f"Broadcast адрес:  {int_to_ip(broadcast)}\n"
        f"Маска сети:       {int_to_ip(netmask)}\n"
        f"MAC-адрес:        {get_mac_address()}"
    )
    output_label.configure(text=output)

# === Интерфейс ===
app = ctk.CTk()
app.title("IP Range Analyzer")
app.geometry("500x400")

label = ctk.CTkLabel(app, text="Введите диапазон IP (например: 192.168.0.1 - 192.168.0.255):")
label.pack(pady=10)

entry = ctk.CTkEntry(app, width=400)
entry.pack(pady=10)

button = ctk.CTkButton(app, text="Рассчитать", command=analyze_ip_range)
button.pack(pady=10)

output_label = ctk.CTkLabel(app, text="", justify="left")
output_label.pack(pady=20)

app.mainloop()
