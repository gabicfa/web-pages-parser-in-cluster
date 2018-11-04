import os

from subprocess import PIPE, run

def out(command):
    result = run(command, stdout=PIPE, stderr=PIPE, universal_newlines=True, shell=True)
    return result.stdout

os.system("make") 

links = [
"https://www.magazineluiza.com.br/tv-monitor/tv-e-video/s/et/emtv/",
"https://www.magazineluiza.com.br/notebook-lenovo-ideapad/informatica/s/in/leip/", 
"https://www.magazineluiza.com.br/dvd-player/tv-e-video/s/et/tvdb/",
"https://www.magazineluiza.com.br/hd-externo/informatica/s/in/hdex/",
"https://www.magazineluiza.com.br/adega/eletrodomesticos/s/ed/adeg/"
]

t_ocioso_dis_3 = []
t_medProd_dis_3 = []
t_total_dis_3 = []

t_ocioso_dis_8 = []
t_medProd_dis_8 = []
t_total_dis_8 = []

t_ocioso_dis_16 = []
t_medProd_dis_16 = []
t_total_dis_16 = []

t_ocioso_par_5_5 = []
t_medProd_par_5_5 = []
t_total_par_5_5 = []

t_ocioso_seq = []
t_medProd_seq = []
t_total_seq = []

t_num_prod = []

f= open("analysis.txt","w+")

for l in links:

    print(l)
    print("Crawler Sequencial sendo feito...")
    comando = "./crawlerSEQ " + l
    json = out(comando)
    with open('../out.txt') as o:
        lines = o.read().splitlines()

    t_ocioso_seq.append(lines[0])
    if(lines[1]!='0'):
        erro=False
        t_num_prod.append(lines[1])
        t_medProd_seq.append(lines[2])
        t_total_seq.append(lines[3])
    else:
        erro = True
        print("ERRO NO SITE MAGAZINE, RODE O SCRIPT NOVAMENTE OU TENTE MAIS TARDE")
        break

    print("Crawler Distribuido sendo feito com 3 processos...")
    comando = "mpiexec -n 3 ./crawlerDIS " + l
    with open('../out.txt') as o:
        lines = o.read().splitlines()

    t_ocioso_dis_3.append(lines[0])
    t_medProd_dis_3.append(lines[2])
    t_total_dis_3.append(lines[3])
    
    print("Crawler Distribuido sendo feito com 8 processos...")
    comando = "mpiexec -n 8 ./crawlerDIS " + l
    json = out(comando)
    with open('../out.txt') as o:
        lines = o.read().splitlines()

    t_ocioso_dis_8.append(lines[0])
    t_medProd_dis_8.append(lines[2])
    t_total_dis_8.append(lines[3])

    print("Crawler Distribuido sendo feito com 16 processos...")
    comando = "mpiexec -n 16 ./crawlerDIS " + l
    json = out(comando)
    with open('../out.txt') as o:
        lines = o.read().splitlines()

    t_ocioso_dis_16.append(lines[0])
    t_medProd_dis_16.append(lines[2])
    t_total_dis_16.append(lines[3])

    # print("Crawler Paralelo sendo feito com 5 threads produtoras e 5 consumidoras...")
    # comando = "./crawlerPAR " + l + " 5 5"
    # json = out(comando)
    # with open('../out.txt') as o:
    #     lines = o.read().splitlines()

    # t_ocioso_par_5_5.append(lines[0])
    # t_medProd_par_5_5.append(lines[2])
    # t_total_par_5_5.append(lines[3])

    
    
    
if(not erro):
    f.write("t_num_prod=[")
    for i in range (0,len(t_num_prod)):
        if(i!=len(t_num_prod)-1):
            f.write(t_num_prod[i] + ',')
        else:
            f.write(t_num_prod[i])
    f.write("]"+ '\n')

    f.write("t_ocioso_seq=[")
    for i in range (0,len(t_ocioso_seq)):
        if(i!=len(t_ocioso_seq)-1):
            f.write(t_ocioso_seq[i] + ',')
        else:
            f.write(t_ocioso_seq[i])
    f.write("]"+ '\n')

    f.write("t_medProd_seq=[")
    for i in range (0,len(t_medProd_seq)):
        if(i!=len(t_medProd_seq)-1):
            f.write(t_medProd_seq[i] + ',')
        else:
            f.write(t_medProd_seq[i])
    f.write("]"+ '\n')

    f.write("t_total_seq=[")
    for i in range (0,len(t_total_seq)):
        if(i!=len(t_total_seq)-1):
            f.write(t_total_seq[i] + ',')
        else:
            f.write(t_total_seq[i])
    f.write("]"+ '\n')

    # f.write("t_ocioso_par_5_5=[")
    # for i in range (0,len(t_ocioso_par_5_5)):
    #     if(i!=len(t_ocioso_par_5_5)-1):
    #         f.write(t_ocioso_par_5_5[i] + ',')
    #     else:
    #         f.write(t_ocioso_par_5_5[i])
    # f.write("]"+ '\n')

    # f.write("t_medProd_par_5_5=[")
    # for i in range (0,len(t_medProd_par_5_5)):
    #     if(i!=len(t_medProd_par_5_5)-1):
    #         f.write(t_medProd_par_5_5[i] + ',')
    #     else:
    #         f.write(t_medProd_par_5_5[i])
    # f.write("]"+ '\n')

    # f.write("t_total_par_5_5=[")
    # for i in range (0,len(t_total_par_5_5)):
    #     if(i!=len(t_total_par_5_5)-1):
    #         f.write(t_total_par_5_5[i] + ',')
    #     else:
    #         f.write(t_total_par_5_5[i])
    # f.write("]"+ '\n')

    f.write("t_ocioso_dis_3=[")
    for i in range (0,len(t_ocioso_dis_3)):
        if(i!=len(t_ocioso_par_5_5)-1):
            f.write(t_ocioso_dis_3[i] + ',')
        else:
            f.write(t_ocioso_dis_3[i])
    f.write("]"+ '\n')

    f.write("t_medProd_dis_3=[")
    for i in range (0,len(t_medProd_dis_3)):
        if(i!=len(t_medProd_dis_3)-1):
            f.write(t_medProd_dis_3[i] + ',')
        else:
            f.write(t_medProd_dis_3[i])
    f.write("]"+ '\n')

    f.write("t_total_dis_3=[")
    for i in range (0,len(t_total_dis_3)):
        if(i!=len(t_total_dis_3)-1):
            f.write(t_total_dis_3[i] + ',')
        else:
            f.write(t_total_dis_3[i])
    f.write("]"+ '\n')

    f.write("t_ocioso_dis_8=[")
    for i in range (0,len(t_ocioso_dis_8)):
        if(i!=len(t_ocioso_dis_8)-1):
            f.write(t_ocioso_dis_8[i] + ',')
        else:
            f.write(t_ocioso_dis_8[i])
    f.write("]"+ '\n')

    f.write("t_medProd_dis_8=[")
    for i in range (0,len(t_medProd_dis_8)):
        if(i!=len(t_medProd_dis_8)-1):
            f.write(t_medProd_dis_8[i] + ',')
        else:
            f.write(t_medProd_dis_8[i])
    f.write("]"+ '\n')

    f.write("t_total_dis_8=[")
    for i in range (0,len(t_total_dis_8)):
        if(i!=len(t_total_dis_8)-1):
            f.write(t_total_dis_8[i] + ',')
        else:
            f.write(t_total_dis_8[i])
    f.write("]"+ '\n')

    f.write("t_ocioso_dis_16=[")
    for i in range (0,len(t_ocioso_dis_8)):
        if(i!=len(t_ocioso_dis_8)-1):
            f.write(t_ocioso_dis_8[i] + ',')
        else:
            f.write(t_ocioso_dis_8[i])
    f.write("]"+ '\n')

    f.write("t_medProd_dis_16=[")
    for i in range (0,len(t_medProd_dis_8)):
        if(i!=len(t_medProd_dis_8)-1):
            f.write(t_medProd_dis_8[i] + ',')
        else:
            f.write(t_medProd_dis_8[i])
    f.write("]"+ '\n')

    f.write("t_total_dis_16=[")
    for i in range (0,len(t_total_dis_8)):
        if(i!=len(t_total_dis_8)-1):
            f.write(t_total_dis_8[i] + ',')
        else:
            f.write(t_total_dis_8[i])
    f.write("]"+ '\n')