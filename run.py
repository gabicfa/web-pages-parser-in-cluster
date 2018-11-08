import os

from subprocess import PIPE, run

# def out(command):
#     result = run(command, stdout=PIPE, stderr=PIPE, universal_newlines=True, shell=True)
#     return result.stdout

# os.system("make") 

links = [
"https://www.magazineluiza.com.br/notebook-lenovo-ideapad/informatica/s/in/leip/",
"https://www.magazineluiza.com.br/dvd-player/tv-e-video/s/et/tvdb/"
# "https://www.magazineluiza.com.br/hd-externo/informatica/s/in/hdex/",
# "https://www.magazineluiza.com.br/adega/eletrodomesticos/s/ed/adeg/"
]

t_ocioso_dis_1 = []
t_medProd_dis_1 = []
t_total_dis_1 = []

t_ocioso_dis_2 = []
t_medProd_dis_2 = []
t_total_dis_2 = []

t_ocioso_dis_3 = []
t_medProd_dis_3 = []
t_total_dis_3 = []

t_ocioso_dis_4 = []
t_medProd_dis_4 = []
t_total_dis_4 = []

t_ocioso_dis_5 = []
t_medProd_dis_5 = []
t_total_dis_5 = []

t_ocioso_dis_6 = []
t_medProd_dis_6 = []
t_total_dis_6 = []

t_ocioso_dis_7 = []
t_medProd_dis_7 = []
t_total_dis_7 = []

t_ocioso_par_3_3 = []
t_medProd_par_3_3 = []
t_total_par_3_3 = []

t_ocioso_seq = []
t_medProd_seq = []
t_total_seq = []

t_num_prod = []

f= open("analysis.txt","w+")

for l in links:

    print(l)
    print("Crawler Sequencial sendo feito...")
    comando = "build/crawlerSEQ " + l
    json = os.system(comando)
    with open('out.txt') as o:
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
    
    print("Crawler Paralelo sendo feito com 3 threads produtoras e 3 consumidoras...")
    comando = "build/crawlerPAR " + l + " 3 3"
    json = os.system(comando)
    with open('out.txt') as o:
        lines = o.read().splitlines()

    t_ocioso_par_3_3.append(lines[0])
    t_medProd_par_3_3.append(lines[2])
    t_total_par_3_3.append(lines[3])

    mpilist = ["4","8","12","16","20","24","28"]
    hstfile = ["host0","host1","host2","host3","host4","host5","host6"]
    mpilistlistocioso = [t_ocioso_dis_1,t_ocioso_dis_2,t_ocioso_dis_3,t_ocioso_dis_4,t_ocioso_dis_5,t_ocioso_dis_6,t_ocioso_dis_7]
    mpilistlistmedProd = [t_medProd_dis_1,t_medProd_dis_2,t_medProd_dis_3,t_medProd_dis_4,t_medProd_dis_5,t_medProd_dis_6,t_medProd_dis_7]
    mpilistlisttotal = [t_total_dis_1, t_total_dis_2, t_total_dis_3,t_total_dis_4,t_total_dis_5,t_total_dis_6, t_total_dis_7]
    
    for m in range (0, len(mpilist)):
        print("Crawler Distribuido sendo feito com " + mpilist[m] + " processos e "+ str(m+1) +" maquinas")
        comando = "mpiexec -n "+ mpilist[m] +" -hostfile hostfiles/"+hstfile[m]+ " build/crawlerDIS " + l #comando para maquinas no cluster
        # comando = "mpiexec -n "+ mpilist[m] +" build/crawlerDIS " + l #comando para maquinas locais
        json = os.system(comando)
        with open('out.txt') as o:
            lines = o.read().splitlines()

        mpilistlistocioso[m].append(lines[0])
        mpilistlistmedProd[m].append(lines[2])
        mpilistlisttotal[m].append(lines[3])



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
            print(t_ocioso_seq[i])
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

    f.write("t_ocioso_par_3_3=[")
    for i in range (0,len(t_ocioso_par_3_3)):
        if(i!=len(t_ocioso_par_3_3)-1):
            f.write(t_ocioso_par_3_3[i] + ',')
        else:
            f.write(t_ocioso_par_3_3[i])
    f.write("]"+ '\n')

    f.write("t_medProd_par_3_3=[")
    for i in range (0,len(t_medProd_par_3_3)):
        if(i!=len(t_medProd_par_3_3)-1):
            f.write(t_medProd_par_3_3[i] + ',')
        else:
            f.write(t_medProd_par_3_3[i])
    f.write("]"+ '\n')

    f.write("t_total_par_3_3=[")
    for i in range (0,len(t_total_par_3_3)):
        if(i!=len(t_total_par_3_3)-1):
            f.write(t_total_par_3_3[i] + ',')
        else:
            f.write(t_total_par_3_3[i])
    f.write("]"+ '\n')

    for m in range (0, len(mpilistlistocioso)):
        s = "t_ocioso_dis_"+str(m+1)
        f.write(s+"=[")
        for i in range (0,len(mpilistlistocioso[m])):
            if(i!=len(mpilistlistocioso[m])-1):
                f.write(mpilistlistocioso[m][i] + ',')
            else:
                f.write(mpilistlistocioso[m][i])
        f.write("]"+ '\n')
    
    for m in range (0, len(mpilistlistmedProd)):
        s = "t_medProd_dis_"+str(m+1)
        f.write(s+"=[")
        for i in range (0,len(mpilistlistmedProd[m])):
            if(i!=len(mpilistlistmedProd[m])-1):
                f.write(mpilistlistmedProd[m][i] + ',')
            else:
                f.write(mpilistlistmedProd[m][i])
        f.write("]"+ '\n')
    
    for m in range (0, len(mpilistlisttotal)):
        s = "t_total_dis_"+str(m+1)
        f.write(s+"=[")
        for i in range (0,len(mpilistlisttotal[m])):
            if(i!=len(mpilistlisttotal[m])-1):
                f.write(mpilistlisttotal[m][i] + ',')
            else:
                f.write(mpilistlisttotal[m][i])
        f.write("]"+ '\n')