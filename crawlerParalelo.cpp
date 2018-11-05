#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <thread>
#include <chrono>
#include <boost/regex.hpp>
#include <curl/curl.h>
#include <curl/easy.h>
#include <fstream>
#include "semaphore.cpp"

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

//Procura em uma string de entrada str uma string ditada pelo regex reg e retorna 
//os matches, de acordo com o index desejado, pelo vetor result
void findMatches(string str, regex reg, list<string>& result, int index){
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
    list<string> lastPage;
    regex numPages("\"lastPage\":([^,]+)");
    findMatches(page, numPages, lastPage, 1);
    int totalPages = stoi(lastPage.back());
    return totalPages;
}

//Dado uma página de produtos page, o total de página e o número da página atual, procura 
//as urls dos produtos presentes nela e a url para a próxima página
list<string> findMatchesPages(string page, int totalPages, int numPag){
    list<string> urlsProducts;
    list<string> lastPage;

    regex href("name=\"linkToProduct\" href=\"([^\"]+)");
    findMatches(page, href, urlsProducts, 1);
    
    if(numPag != totalPages){
        regex nextPage("<link rel=\"next\" href=\"([^\"]+)");
        findMatches(page, nextPage, lastPage, 1);
        if(lastPage.size()>0){
            string next = lastPage.front();
            next = "https://www.magazineluiza.com.br"+next;
            urlsProducts.push_back(next);
        }
        return urlsProducts;
    }
    //Caso seja a última página, não tem próxima página
    else{
        urlsProducts.push_back("none");
        return urlsProducts;
    }
}

//A partir da página de produto page é possivel extrair as informações necessárias 
string collectProduct(string page){
    list<string> buffer;
    string productName;
    string productDescription;
    string productImage;
    string productPrice;
    string precoParcelado;
    string numeroParcelas;
    string productCategory;
    string productUrl;

    regex name ("<h1 class=\"header-product__title\" itemprop=\"name\">([^<]+)");
    findMatches(page, name, buffer, 1);
    if(buffer.size()>0){
        productName = buffer.front();
    }
    else{
        productName = " ";
    } 

    buffer.clear();
    regex desc("<h2 class=\"description__product-title\">([^<]+)</h2>    <p class=\"description__text\"></p>([^<]+)");
    findMatches(page, desc, buffer, 2);
    if(buffer.size()>0){
        productDescription = buffer.front();
    }
    else{
        productDescription = " ";
    } 

    buffer.clear();
    regex image("showcase-product__big-img js-showcase-big-img\" src=\"(https[^\"]+)");
    findMatches(page, image, buffer, 1);
    if(buffer.size()>0){
        productImage = buffer.front();
    }
    else{
        productImage = " ";
    } 

    buffer.clear();
    regex price("price-template__text[^>]+>([^<]+)</span>");
    findMatches(page, price, buffer, 1);
    if(buffer.size()>0){
        productPrice = buffer.front();
    }
    else{
        productPrice = "0";
    } 
    
    buffer.clear();
    regex parcelado("installmentAmount\": \" ([^\"]+)");
    findMatches(page, parcelado, buffer, 1);
    if(buffer.size()>0){
        precoParcelado = buffer.front();
    }
    else{
        precoParcelado = "0";
    } 

    buffer.clear();
    regex numparcelas("installmentQuantity\": \"([^\"]+)");
    findMatches(page, numparcelas, buffer, 1);
    if(buffer.size()>0){
        numeroParcelas = buffer.front();
    }
    else{
        numeroParcelas = "0";
    } 

    buffer.clear();
    regex category("itemprop=\"item\"> ([^>]+)</a>  </li>  </ul>");
    findMatches(page, category, buffer, 1);
    if(buffer.size()>0){
        productCategory = buffer.front();
    }
    else{
        productCategory = " ";
    }
    buffer.clear();
    regex url("socialShare[^u]+url: \'([^\']+)");
    findMatches(page, url, buffer, 1);
    if(buffer.size()>0){
        productUrl = buffer.front();
    }
    else{
        productUrl = " ";
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
    "    \"url\" : \"" + productUrl +"\",\n"
    "  },\n";
    return out;
}
/*FUNÇÃO de prodCollectorThread - coleta uma pagina de listofPages, coleta as informações 
desejadas e as adiciona a string global "finalJSON"*/
void consumePages(list<string>& listofPages, Semaphore& accessListofPages, Semaphore& listofPagesCount, Semaphore& accessJSON, const string &finalJSON, bool& noMorePages, int numConsumers){
    duration<double> tempoProd;
    high_resolution_clock::time_point t1, t2;

    string currentProductPage;
    string jsonProduct;
    double ociosoProd;
    string* tempJson = const_cast<string*>(&finalJSON);
    bool end = false;
    list<string> buffer;

    while(!end){
        listofPagesCount.acquire();
        accessListofPages.acquire();
            ociosoProd=0;
            if(noMorePages && listofPages.empty()){
                end = true;
            }
            else{
                currentProductPage = listofPages.front();
                listofPages.pop_front();
                if(noMorePages && listofPages.empty()){
                    for(int ct=0; ct< numConsumers; ct++){
                        listofPagesCount.release();
                    }
                }
            };
        accessListofPages.release();

        regex tempoociosoprod ("DownloadTime=([^>]+)>");
        findMatches(currentProductPage, tempoociosoprod, buffer, 1);
        if(buffer.size()> 0){
            ociosoProd= stod(buffer.front());
        }

        t1 = high_resolution_clock::now();
            jsonProduct = collectProduct(currentProductPage);
        t2 = high_resolution_clock::now();
        tempoProd = duration_cast<duration<double> >(t2 - t1);

        cerr << "Tempo gasto no produto: " << tempoProd.count() + ociosoProd << '\n'; //Tempo gasto no produto
        
        accessJSON.acquire();
            *tempJson = finalJSON + jsonProduct;
        accessJSON.release();
    }
}

/*FUNÇÃO de prodPageCollectorThread - Recolhe* uma url da listofUrls, faz o 
download dela e a adicionar à listofPages*/
void producePages(list<string>&listofUrls, Semaphore& accessListofUrls, Semaphore& listofUrlCount, list<string>&listofPages, Semaphore& accessListofPages, Semaphore& listofPagesCount, bool& noMoreUrls, bool& noMorePages, int numProducers, Semaphore& accesstempoOcioso, double& tempoOcioso){
    string currentProductUrl;
    string currentProductPage;
    high_resolution_clock::time_point t1, t2;
    duration<double> ocioso;
    bool end = false;
    while(!end){
        listofUrlCount.acquire();
        accessListofUrls.acquire();
            if(noMoreUrls && listofUrls.empty()){
                end = true;
            }
            else{
                currentProductUrl = listofUrls.front();
                listofUrls.pop_front();
                if(noMoreUrls && listofUrls.empty()){
                    for(int pt=0; pt< numProducers; pt++){
                        listofUrlCount.release();
                    }
                }
            }
        accessListofUrls.release();

        t1 = high_resolution_clock::now();
            currentProductPage = download(currentProductUrl);
        t2 = high_resolution_clock::now();
        ocioso = duration_cast<duration<double> >(t2 - t1);

        accesstempoOcioso.acquire();
            tempoOcioso += ocioso.count();
            currentProductPage+="DownloadTime="+to_string(ocioso.count())+">";
        accesstempoOcioso.release();

        accessListofPages.acquire();
            listofPages.push_back(currentProductPage);
            listofPagesCount.release();
        accessListofPages.release();
    }
    noMorePages = true;
}

/*FUNÇÃO de prodLinkCollectorThread -  Faz o download das páginas com os produtos, identifica as urls 
dos produtos presentes nelas e coloca-as na listofUrls. Além disso adquire a url da pagina com 
produtos seguinte e fazer o mesmo processo, sucessivamente, até a última página dessa categoria.*/
void produceUrls(list<string>& listofUrls, Semaphore& accessListofUrls, Semaphore& listofUrlCount, string url, bool& noMoreUrls, Semaphore& accesstempoOcioso, double& tempoOcioso, double& numProd){
    high_resolution_clock::time_point t1, t2;
    duration<double> ocioso;
    
    list<string> urlsPage;
    string nextPageUrl;
    
    t1 = high_resolution_clock::now();
        string page = download(url);
    t2 = high_resolution_clock::now();
    ocioso = duration_cast<duration<double> >(t2 - t1);
    accesstempoOcioso.acquire();
        tempoOcioso += ocioso.count();
    accesstempoOcioso.release();
    
    
    int total = totalPages(page);

    for(int p=1; p<=total; p++){
        cout << "pagina " << p << "/" << total << '\n';
        urlsPage = findMatchesPages(page, total, p);
        if(urlsPage.size()>0){
            nextPageUrl = urlsPage.back();
            cout << nextPageUrl << '\n';
            urlsPage.pop_back();
            numProd+=urlsPage.size();
            accessListofUrls.acquire();
                for(auto u = urlsPage.begin(); u != urlsPage.end(); ++u){
                    listofUrls.push_back(*u);
                    listofUrlCount.release();
                }
            accessListofUrls.release();
            

            t1 = high_resolution_clock::now();
                page = download(nextPageUrl);
            t2 = high_resolution_clock::now();
            ocioso = duration_cast<duration<double> >(t2 - t1);
            accesstempoOcioso.acquire();
                tempoOcioso += ocioso.count();
            accesstempoOcioso.release();
        }
        else{
            cerr << "Erro ao carregar pagina " << p << "\n";
            break;
        }
    }
    noMoreUrls = true;
}

int main(int argc, char *argv[]) {
    high_resolution_clock::time_point total1, total2;
    duration<double> total;
    total1 = high_resolution_clock::now();

    Semaphore accessListofUrls(1);
    Semaphore accessListofPages(1);
    Semaphore accessJSON(1);
    Semaphore listofUrlCount(0);
    Semaphore listofPagesCount(0);

    list<string> listofUrls;
    list<string> listofPages;

    Semaphore accesstempoOcioso(1);
    double tempoOcioso=0;
    double numProd=0;

    bool noMorePages = false;
    bool noMoreUrls = false;

    string finalJSON = "[\n";

    string url = argv[1];
    int numProducers = atoi(argv[2]);
    int numConsumers = atoi(argv[3]);

    thread prodPageCollectorThread[numProducers];
    thread prodCollectorThread[numConsumers];

    thread prodLinkCollectorThread(produceUrls, std::ref(listofUrls), std::ref(accessListofUrls), std::ref(listofUrlCount), url, std::ref(noMoreUrls), std::ref(accesstempoOcioso), std::ref(tempoOcioso), std::ref(numProd));
    for(int p=0; p<numProducers; p++){
        prodPageCollectorThread[p] = thread(producePages, std::ref(listofUrls), std::ref(accessListofUrls), std::ref(listofUrlCount), std::ref(listofPages), std::ref(accessListofPages), std::ref(listofPagesCount), std::ref(noMoreUrls), std::ref(noMorePages), numProducers, std::ref(accesstempoOcioso), std::ref(tempoOcioso));
    }
    for(int c=0; c<numConsumers; c++){
        prodCollectorThread[c] = thread(consumePages,  std::ref(listofPages), std::ref(accessListofPages), std::ref(listofPagesCount), std::ref(accessJSON), std::ref(finalJSON), std::ref(noMorePages), numConsumers);
    }

    prodLinkCollectorThread.join();
    for(int p=0; p<numProducers; p++){
        prodPageCollectorThread[p].join();
    }
    for(int c=0; c<numConsumers; c++){
        prodCollectorThread[c].join();
    }

    finalJSON += "]\n";
    cout << finalJSON;

    ofstream myfile;
    myfile.open ("../outPAR.txt");
    myfile << tempoOcioso << '\n';
    myfile << numProd << '\n';

    total2 = high_resolution_clock::now();
    total = duration_cast<duration<double> >(total2 - total1);

    myfile << total.count()/numProd << '\n';
    myfile << total.count() << '\n';
    myfile.close();

    return 0;
}