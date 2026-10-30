# Child

Uma base **virgem** de rede neural, escrita em C++20 sem dependências externas, pensada para aprender e **ramificar**.

## A ideia

A maioria das IAs de hoje é treinada uma vez, congelada e depois só usada. O aprendizado humano é diferente: contínuo, acumulativo e cada pessoa segue seu caminho a partir do que viveu.

Child é o ponto de partida de um experimento nessa direção: uma rede sem nada aprendido, capaz de ser copiada em ramificações que **divergem** conforme o que cada uma estuda, sem afetar a origem nem as irmãs. A meta de longo prazo é aproximar o aprendizado da rede do humano. Este repositório cobre só a **base**.

## O que existe hoje

Três camadas, nada mais:

| Camada | O que faz |
|---|---|
| **Captação** | Lê um arquivo, uma pasta ou stdin e converte em bytes. Cada byte é um token, então serve para qualquer texto ou código, sem tokenizador para treinar. |
| **Processamento** | Rede que olha os 16 bytes anteriores e prevê o próximo. Passagem direta, retropropagação e SGD, tudo escrito do zero. |
| **Armazenamento** | Checkpoint binário com checksum e escrita atômica, que guarda os pesos e a **linhagem**: id, pai, nome e quantos bytes já aprendeu. |

A **ramificação** (`fork`) copia o estado inteiro e dá à cópia uma identidade própria apontando para o pai.

## O que ainda não é

Seja realista: é uma rede minúscula (cerca de 107 mil parâmetros) com janela fixa de 16 bytes e sem memória além dela. Treinada em um pouco de código, ela imita o formato e gera texto sem sentido. Ela **não** tem capacidade comparável à de um adulto, e não há proteção contra o esquecimento catastrófico ao alimentar uma ramificação com dados novos. A base existe para crescer: os próximos passos naturais são uma arquitetura com atenção, aprendizado contínuo e treino mais rápido.

## Como usar

Precisa de um compilador C++20 e CMake.

```bash
cmake -S . -B build -G Ninja && cmake --build build   # compilar
ctest --test-dir build                                # testes

./build/child init root.child --name root             # base virgem
./build/child fork root.child a.child --name a        # ramificação
./build/child feed a.child src --epochs 3             # captar e aprender
./build/child info a.child                            # linhagem e estado
./build/child sample a.child --prompt "int "          # gerar texto
```

Depois do `feed`, só `a.child` terá bytes aprendidos; `root.child` continua virgem.

## Documentação

- [Arquitetura](.centaur/system/architecture.md): fluxo, módulos e formato do checkpoint.
- [Contrato da base](.centaur/contracts/base/v002.json): o comportamento esperado, regra por regra, com as [evidências](.centaur/evidence) dos testes que o comprovam.
- [AGENTS.md](AGENTS.md): instruções para agentes de IA que trabalham no projeto, que usa o fluxo [Centaur](.centaur/workspace.json) para contratos e evidências.
