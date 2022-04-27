[![Website](https://img.shields.io/website?up_message=tentris&url=https%3A%2F%2Ftentris.dice-research.org%2F)](https://tentris.dice-research.org/) [![Build Status](https://travis-ci.com/dice-group/tentris.svg)](https://travis-ci.com/dice-group/tentris) [![Docker Cloud Build Status](https://img.shields.io/docker/cloud/build/dicegroup/tentris_server)](https://hub.docker.com/repository/docker/dicegroup/tentris_server) ![GitHub](https://img.shields.io/github/license/dice-group/tentris)  

# Tᴇɴᴛʀɪs: A Tensor-based Triple Store 

<p><img src = "https://tentris.dice-research.org/iswc2020/assets/img/Tentris_logo.svg" alt = "Tᴇɴᴛʀɪs Logo" width = "30%" align = "center"></p>

Tᴇɴᴛʀɪs is a tensor-based RDF triple store with SPARQL support. It is introduced and described in:
> Alexander Bigerl, Felix Conrads, Charlotte Behning, Mohamed Ahmed Sherif, Muhammad Saleem and Axel-Cyrille Ngonga Ngomo (2020) **Tentris – A Tensor-Based Triple Store.** In: The Semantic Web – ISWC 2020

https://tentris.dice-research.org/iswc2020/

```
@InProceedings{bigerl2020tentris,
  author = {Bigerl, Alexander and Conrads, Felix and Behning, Charlotte and Sherif, Mohamed Ahmed and Saleem, Muhammad and Ngonga Ngomo, Axel-Cyrille},
  booktitle = {The Semantic Web -- ISWC 2020},
  publisher = {Springer International Publishing},
  title = { {T}entris -- {A} {T}ensor-{B}ased {T}riple {S}tore},
  pages = {56--73},
  url = {https://papers.dice-research.org/2020/ISWC_Tentris/iswc2020_tentris_public.pdf},
  year = 2020,
  isbn = {978-3-030-62419-4}
}
```


## Key Features
* fast tensor-based storage and query processing
* SPARQL Protocol conform HTTP interface
* supports at the moment SPARQL queries with SELECT + opt. DISTINCT + basic graph pattern
* available for Linux x86-64 

<details><summary>Current limitations: </summary>

* SPARQL support limited to SELECT + opt. DISTINCT + basic graph pattern
* data loading only possible at startup

</details>

## Get It
* download [static prebuilt binaries](https://github.com/dice-group/tentris/releases) and [try them out](#running-tentris)
* pull a [docker image](https://hub.docker.com/repository/docker/dicegroup/tentris_server) and [try them out](#docker)
* [build it yourself](#build-it-yourself)

## Running Tᴇɴᴛʀɪs 
<details><summary> </summary>

#### Bulk-load Data
Provide an NTRIPLE or TURTLE file to build the an index. By default, the index is stored in the current directory. The path can be changed with the option `--storage`. 
```shell
tentris_loader --file my_nt_file.nt
```
#### Start HTTP endpoint
To start Tᴇɴᴛʀɪs as a HTTP endpoint on port 9080 run now: 
```
tentris_server -p 9080
``` 

#### Query
The SPARQL endpoint may now be queried locally at: `127.0.0.1:9080/sparql?query=*your query*`. You can execute queries with the following curl command:  
```shell
curl -G \
--data-urlencode 'query=SELECT * WHERE { ?s ?p ?o . }' \
'127.0.0.1:9080/sparql'
```
If you want to type the query in your browser, the query string must be URL encoded. You can use any online URL encoder like <https://meyerweb.com/eric/tools/dencoder>.

The following endpoints are available:
Available endpoints:
- HTTP GET `/sparql?query=` for normal queries
- HTTP GET `/stream?query=` for queries with huge results
- HTTP GET `/count?query=` as a workaround for count (consumes a select query)
- HTTP GET `/ask?query=` as a workaround for ask (consumes a select query)

</details>


## Docker
<details><summary> </summary>
  
Using the Tᴇɴᴛʀɪs docker image is really easy. 

* A docker image is available on [docker hub](https://hub.docker.com/r/dicegroup/tentris_server). Get it with 
    ```shell script
    docker pull dicegroup/tentris_server
    ```
TODO: Describe how to load data and start the endpoint.
  
</details>

## Build It Yourself
<details><summary> </summary>
  
Tᴇɴᴛʀɪs is known to build on Ubuntu 22.04 and newer.
Building was tested with GCC 11, Clang 13 and Clang 14. As standard library, only libstdc++11 was tested. For details refer to the [Dockerfile](./Dockerfile).
  
</details>
