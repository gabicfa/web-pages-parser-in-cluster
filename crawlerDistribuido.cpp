#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
#include <boost/regex.hpp>
#include <chrono>
#include <curl/curl.h>
#include <curl/easy.h>
#include <sstream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace std::chrono;


//função auxiliar para a funcao "download"
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/*Faz o download de uma pagina web a partir de sua url e retorna o seu conteúdo 
html em formato de string*/
string download(string url) {
    CURL *curl;
    string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return readBuffer;
}

/*Procura em uma string de entrada str uma string ditada pelo regex reg e retorna 
os matches, de acordo com o index desejado, pelo vetor result*/
void findMatches(string str, regex reg, vector<string>& result, int index){
    smatch matches;
    while(regex_search(str, matches, reg)){
        //caso exista um match
        if(matches.size()> 0){
            result.push_back(matches[index]);
            str = matches.suffix().str(); 
        }
    }
}

//Busca o total de páginas de produtos daquela categoria a partir da primeira página page
int totalPages(string page){
    vector<string> lastPage;
    regex numPages("\"lastPage\":([^,]+)");
    findMatches(page, numPages, lastPage, 1);
    int totalPages = stoi(lastPage[0]);
    return totalPages;
}

/*Dado uma página de produtos page, o total de página e o número da página atual, procura 
as urls dos produtos presentes nela e a url para a próxima página*/
vector<string> findMatchesPages(string page, int totalPages, int numPag){
    vector<string> urlsProducts;
    vector<string> lastPage;

    regex href("name=\"linkToProduct\" href=\"([^\"]+)");
    findMatches(page, href, urlsProducts, 1);

    if(numPag != totalPages){
        regex nextPage("<link rel=\"next\" href=\"([^\"]+)");
        findMatches(page, nextPage, lastPage, 1);
        if(lastPage.size()>0){
            lastPage[0] = "https://www.magazineluiza.com.br"+lastPage[0];
            urlsProducts.push_back(lastPage[0]);
        }
        return urlsProducts;
    }
    //Caso seja a última página, não tem próxima página
    else{
        urlsProducts.push_back("none");
        return urlsProducts;
    }
}

/*A partir da página de produto page é possivel extrair as informações necessárias, 
a url do produto já é informação adquirida anteriormente*/
string collectProduct(string page, string url){
    vector<string> buffer;
    string productName;
    string productDescription;
    string productImage;
    string productPrice;
    string precoParcelado;
    string numeroParcelas;
    string productCategory;

    regex name ("<h1 class=\"header-product__title\" itemprop=\"name\">([^<]+)");
    findMatches(page, name, buffer, 1);
    if(buffer.size()>0){
        productName = buffer[0];
    }
    else{
        productName = " ";
    } 

    buffer.clear();
    regex desc("<h2 class=\"description__product-title\">([^<]+)</h2>    <p class=\"description__text\"></p>([^<]+)");
    findMatches(page, desc, buffer, 2);
    if(buffer.size()>0){
        productDescription = buffer[0];
    }
    else{
        productDescription = " ";
    } 

    buffer.clear();
    regex image("showcase-product__big-img js-showcase-big-img\" src=\"(https[^\"]+)");
    findMatches(page, image, buffer, 1);
    if(buffer.size()>0){
        productImage = buffer[0];
    }
    else{
        productImage = " ";
    } 

    buffer.clear();
    regex price("price-template__text[^>]+>([^<]+)</span>");
    findMatches(page, price, buffer, 1);
    if(buffer.size()>0){
        productPrice = buffer[0];
    }
    else{
        productPrice = "0";
    } 
    
    buffer.clear();
    regex parcelado("installmentAmount\": \" ([^\"]+)");
    findMatches(page, parcelado, buffer, 1);
    if(buffer.size()>0){
        precoParcelado = buffer[0];
    }
    else{
        precoParcelado = "0";
    } 

    buffer.clear();
    regex numparcelas("installmentQuantity\": \"([^\"]+)");
    findMatches(page, numparcelas, buffer, 1);
    if(buffer.size()>0){
        numeroParcelas = buffer[0];
    }
    else{
        numeroParcelas = "0";
    } 

    buffer.clear();
    regex category("itemprop=\"item\"> ([^>]+)</a>  </li>  </ul>");
    findMatches(page, category, buffer, 1);
    if(buffer.size()>0){
        productCategory = buffer[0];
    }
    else{
        productCategory = " ";
    }

    string out = 
    "  {\n"
    "    \"nome\" : \"" + productName +"\",\n"
    "    \"descricao\" : \"" + productDescription +"\",\n"
    "    \"foto\" : \"" + productImage +"\",\n"
    "    \"preco\" : \"" + productPrice +"\",\n"
    "    \"preco_parcelado\" : \"" + precoParcelado +"\",\n"
    "    \"preco_num_parcelas\" : \"" + numeroParcelas +"\",\n"
    "    \"categoria\" : \"" + productCategory +"\",\n"
    "    \"url\" : \"" + url +"\",\n"
    "  },\n";

    return out;
}

int main(int argc, char** argv) {

    high_resolution_clock::time_point total1, total2;
    duration<double> tempoTotal;
    total1 = high_resolution_clock::now();

    string url = argv[1];

    boost::mpi::environment env(argc, argv);   
    boost::mpi::communicator world;

    int n = world.size();
    
    if(world.rank()==0){
        
        vector<string> urls;
        string productPage;
        string nextPageUrl;
        int numProdTotal=0;
        int processoAtual = 1; // primeiro processo a enviar é o processo 1

        high_resolution_clock::time_point t1, t2, t3;
        duration<double> ocioso;
        duration<double> tempoProd;
        double tempoOcioso=0;
        
        t1 = high_resolution_clock::now();
            string currentPage = download(url);
        t2 = high_resolution_clock::now();
        ocioso = duration_cast<duration<double> >(t2 - t1);
        if(ocioso.count()>0)tempoOcioso += ocioso.count();
        
        int total = totalPages(currentPage);
        for(int p=1; p<=total; p++){
            cout << "pagina " << p << "/" << total << '\n';
            urls = findMatchesPages(currentPage, total, p);
            if(urls.size()>0){
                nextPageUrl = urls[urls.size()-1];
                urls.pop_back();
            }
            else{
                cerr << "Erro ao carregar pagina " << p << "\n";
                break;
            }

            for(unsigned int u=0; u<urls.size(); u++){
                world.send(processoAtual, 0, urls[u]); // envia para o processoAtual a url[u]
                numProdTotal+=1;
                processoAtual+=1;
                if(processoAtual == n-1){//caso o processo n-2 tenha sido o último a receber a url
                    processoAtual = 1;//o próximo a receber é o processo 1
                } 
            }

            t1 = high_resolution_clock::now();
                currentPage = download(nextPageUrl);
            t2 = high_resolution_clock::now();
            ocioso = duration_cast<duration<double> >(t2 - t1);
            if(ocioso.count()>0)tempoOcioso += ocioso.count();
        }//acabou as páginas contendo produtos

        for (int p = 1; p < n-1; p++){
            string end = ("end");
            world.send(p, 0, end);//envia para os processos de 1 a n-2 a string "end"
        }
        
        world.send(n-1, 2, tempoOcioso);//envia para o processo n-1 medidas de desempenho
        world.send(n-1, 1, numProdTotal);  
    }

    else if (world.rank()!= n-1){//processos de 1 a n-2

        high_resolution_clock::time_point t1, t2, t3;

        duration<double> ocioso;
        duration<double> tempoProd;
        double tempoOcioso=0;

        while(true){//loop para receber mensagens
            string productPage;
            string url;
            world.recv(0,0,url);//recebe de processo 0 url do produto

            if(url.compare("end")==0){//se url for string de finalização
                string out = "end";
                world.send(n-1, 0, out);//envia para processo n-1 string "end"
                world.send(n-1, 2, tempoOcioso);//envia para processo n-1 medida de desempenho tempoOcioso
                break;// termina loop, não irá receber mais mensagens
            }

            t1 = high_resolution_clock::now();
                productPage = download(url);
            t2 = high_resolution_clock::now();
            ocioso = duration_cast<duration<double> >(t2 - t1);
            if(ocioso.count()>0)tempoOcioso += ocioso.count();            

            if(productPage.size()>0){
                string out = collectProduct(productPage, url);
                t3 = high_resolution_clock::now();
                tempoProd = duration_cast<duration<double> >(t3 - t1);
                cerr << "Tempo gasto no produto: " << to_string(tempoProd.count())<<'\n';
                world.send(n-1, 0, out);//envia string out com informações do produto em formato json para processo n-1
            }
        } 
    }

    else{
        int countEnd = 0;//contador de quantos processos ja acabaram
        int numProdTotal;
        int processoAtual = 1;
        double tempoOcioso;
        double tempoOciosoTotal=0;
        string out;
        string jsonFinal ="";

        while(true){//loop para receber mensagens
            
            world.recv(processoAtual, 0, out);

            if(out.compare("end")==0){//se out for string de finalização
                countEnd+=1;//aumenta contador de processos finalizados
                world.recv(processoAtual, 2, tempoOcioso);//recebe tempoOcioso desse processo que finalizou
                tempoOciosoTotal+=tempoOcioso;//adiciona valor recebido a tempoOciosoTotal
                if(countEnd>=n-2){//se todos os processos finalizaram
                    world.recv(0, 1, numProdTotal);//recebe de processo 0 numProdTotal
                    world.recv(0, 2, tempoOcioso);//recebe de processo 0 tempoOcioso
                    tempoOciosoTotal+=tempoOcioso;//adiciona valor recebido a tempoOciosoTotal
                    break;// termina loop, não irá receber mais mensagens
                }
            }

            else{
                jsonFinal +=out;//adiciona string out recebida a string jsonFinal
            }

            processoAtual+=1;
            if(processoAtual == n-1){//caso o processo n-2 tenha sido o último a enviar a string out
                processoAtual = 1;//o próximo a receber é do processo 1
            } 
        }
    
        cout << jsonFinal << '\n';//adiciona string out recebida a string jsonFinal

        ofstream myfile;
        myfile.open ("../out.txt");//criação de arquivo com medidas de desempenho
        myfile << tempoOciosoTotal << '\n';
        myfile << numProdTotal << '\n';

        total2 = high_resolution_clock::now();
        tempoTotal = duration_cast<duration<double> >(total2 - total1);

        myfile << tempoTotal.count()/numProdTotal << '\n';
        myfile << tempoTotal.count() << '\n';
        myfile.close();
    }
}