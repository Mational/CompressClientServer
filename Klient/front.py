from tkinter import *
import os
from tkinter import filedialog
import webbrowser

params = ""
param1 = ""
param2 = ""
param3 = ""
param4 = ""
param5 = ""
param6 = []

def sprawdz_adres():
    a = 0
    b = 0
    c = 0
    d = 0
    LabelError = Label(root, text="                     ", font=('calibre', 10, 'bold'))
    LabelError.grid(row=0, column=3, sticky=W, pady=10, padx=10)
    tmp = 0
    adres = entryIp.get()
    adres_lista = adres.split(".")
    poprawny = True
    if len(adres_lista) != 4:
        poprawny = False
    else:
        for element in adres_lista:
            if (not element.isdigit()) or int(element) > 255 or int(element) < 0:
                poprawny = False
                break
    if poprawny:
        # print(entryIp.get())
        a = entryIp.get()

    else:
        # print("Adres jest niepoprawny")
        tmp += 1

    try:
        liczba = int(entryPort.get())
        if liczba >= 1 and liczba <= 9999:
            #    print(entryPort.get())
            b = entryPort.get()
        else:
            #    print("Wprowadzono niepoprawne dane")
            tmp += 1
    except ValueError:
        # print("Wprowadzono niepoprawne dane")
        tmp += 1

    selection = listbox.curselection()
    if selection:
        if listbox.get(selection[0]) == "Zapis":
            #    print("1")
            c = "1"
        if listbox.get(selection[0]) == "Wysyłka":
            #    print("2")
            c = "2"
        if listbox.get(selection[0]) == "Zapis + Wysyłka":
            #    print("3")
            c = "3"
    else:
        # print("Nie wybrano trybu")
        tmp += 1

    try:
        liczba = int(entryIoscPlikow.get())
        if liczba >= 1 and liczba <= 100:
            #    print(entryIoscPlikow.get())
            d = entryIoscPlikow.get()
        else:
            #    print("Wprowadzono niepoprawne dane")
            tmp += 1
    except ValueError:
        # print("Wprowadzono niepoprawne dane")
        tmp += 1

    if tmp > 0:
        LabelError = Label(root, text='Błędne dane', fg='red', font=('calibre', 10, 'bold'))
        LabelError.grid(row=0, column=3, sticky=W, pady=10, padx=10)
        LabelError.config(text="Błędne dane!")
    else:
        #print(a + " " + b + " " + c + " " + d)
        param1 = a
        param2 = b
        param3 = c
        param5 = d
        params = param1 + " " + param2 + " " + param3 + " " + param4 + " " + param5
        for i in param6:
            params += " " + i
        if param4 == "" or len(param6)==0:
            LabelError = Label(root, text='Błędne dane', fg='red', font=('calibre', 10, 'bold'))
            LabelError.grid(row=0, column=3, sticky=W, pady=10, padx=10)
            LabelError.config(text="Błędne dane!")
        else:
            print(params)
        directory = os.path.realpath(os.path.dirname(__file__))
        print(directory)
        command = directory + "\client.exe " + params
        os.system(command)


def open_explorer():
    plik = filedialog.askopenfilename(filetypes=[("Text Files", "*.zip")])
    plik1 = os.path.basename(plik)
    #print(plik)
    global param4
    param4 = plik
    LabelNazwy.config(text=plik1)


def open_explorer1():
    plik = filedialog.askopenfilename(filetypes=[("Text Files", "*.*")])
    plik1 = os.path.basename(plik)
    #print(plik)
    global param6
    param6.append(plik)
    LabelNazwy1.config(text=LabelNazwy1.cget("text") + "\n" + plik1)


def web():
    webbrowser.open("https://github.com/Mational/CompressClientServer")


root = Tk()
root.geometry("550x350")
root.title("KompresjaApp")

menubar = Menu(root)
filemenu = Menu(menubar, tearoff=0)

filemenu.add_command(label="Kod źródłowy", command=web)
filemenu.add_separator()
filemenu.add_command(label="Zamknij", command=root.quit)

menubar.add_cascade(label="Info", menu=filemenu)

root.config(menu=menubar)

LabelIp = Label(root, text='Wpisz IP:', font=('calibre', 10, 'bold'))
LabelIp.grid(row=0, column=0, sticky=W, pady=10, padx=10)
entryIp = Entry(root)
entryIp.grid(row=0, column=1, sticky=W, pady=10, padx=10)
buttonIp1 = Button(root, text="Dodaj", width=15, command=sprawdz_adres)
buttonIp1.grid(row=5, column=0, sticky=W, pady=2, padx=2)

LabelPort = Label(root, text='Wpisz Port:', font=('calibre', 10, 'bold'))
LabelPort.grid(row=1, column=0, sticky=W, pady=10, padx=10)
entryPort = Entry(root)
entryPort.grid(row=1, column=1, sticky=W, pady=10, padx=10)

LabelTryb = Label(root, text='Wybierz tryb:', font=('calibre', 10, 'bold'))
LabelTryb.grid(row=2, column=0, sticky=W, pady=10, padx=10)
listbox = Listbox(root, width=20, heigh=3)
listbox.insert(END, "Zapis")
listbox.insert(END, "Wysyłka")
listbox.insert(END, "Zapis + Wysyłka")
listbox.grid(row=2, column=1, sticky=W, pady=10, padx=10)

LabelZip = Label(root, text='Wypierz plik:', font=('calibre', 10, 'bold'))
LabelZip.grid(row=3, column=0, sticky=W, pady=10, padx=10)
explore_button = Button(root, text="Eksplorator plików", width=16, command=open_explorer)
explore_button.grid(row=3, column=1, sticky=W, pady=10, padx=10)
LabelNazwy = Label(root, font=('calibre', 10, 'bold'))
LabelNazwy.grid(row=3, column=2, sticky=W, pady=10, padx=10)

LabelIoscPlikow = Label(root, text='Liczba plików:', font=('calibre', 10, 'bold'))
LabelIoscPlikow.grid(row=4, column=0, sticky=W, pady=10, padx=10)
entryIoscPlikow = Entry(root)
entryIoscPlikow.grid(row=4, column=1, sticky=W, pady=10, padx=10)
explore_button = Button(root, text="Eksplorator plików", width=16, command=open_explorer1)
explore_button.grid(row=4, column=2, sticky=W, pady=10, padx=10)
LabelNazwy1 = Label(root, font=('calibre', 10, 'bold'))
LabelNazwy1.grid(row=4, column=3, sticky=W, pady=10, padx=10)

root.mainloop()
