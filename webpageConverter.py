mystr = ""
with open("webpage.html", "r", encoding="utf-8") as fil:
    for satir in fil:
            if len(satir) >=2:
                satir = satir.strip("\n")
                satir = satir.replace("\"", "'")
                satir = f'client.println("{satir}");\n'
            else:
                satir = 'client.println();\n'

            mystr += satir
# print(mystr)
with open("copyThat.txt", "w", encoding="utf-8") as fil:
    fil.writelines(mystr)
