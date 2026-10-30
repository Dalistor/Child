# Pesquisa: do MLP a uma versão simplificada do cérebro

Levantamento de 2026-10-30 que fundamenta o contrato [`cerebro`](../.centaur/contracts/cerebro/v001.json). Baseado nos resumos devolvidos pelas buscas, não nos artigos completos; os números de custo são antigos e dependem da implementação.

## O neurônio humano

- Um modelo detalhado de neurônio piramidal da camada 5 precisou de uma rede profunda de 5 a 8 camadas para ser imitado, e a complexidade vem das condutâncias NMDA dos dendritos. Sem NMDA, uma unidade simples bastava ([Beniaguev, Segev & London](https://www.biorxiv.org/content/10.1101/613141v2)). É um resultado de modelagem sobre um modelo composto, não de medição.
- Em neurônios humanos da camada 2/3, picos dendríticos de cálcio permitem que **um único neurônio resolva XOR** ([Gidon et al., Science 2020](https://dendrites.gr/?p=113)). O código do modelo, em NEURON, está na [ModelDB 260178](https://modeldb.science/260178). Não procuramos réplicas.
- Conclusão: o que torna um neurônio "humano" são os **compartimentos dendríticos**, não o disparo.

## Escada de fidelidade

| Modelo | Fidelidade | Custo relativo |
|---|---|---|
| LIF | mais simples; vazamento do potencial | ~5 operações/ms |
| Izhikevich | reproduz vários padrões de disparo | ~13 operações/ms |
| AdEx | duas equações; prevê com boa precisão o tempo dos disparos do Hodgkin-Huxley ([Brette & Gerstner](https://pmc.ncbi.nlm.nih.gov/articles/PMC2798047)) | próximo ao Izhikevich |
| Hodgkin-Huxley | canais iônicos | ~1.200 operações/ms ([comparativo](https://arxiv.org/pdf/1109.2788)); [um estudo de 2020](https://vufind.lboro.ac.uk/PrimoRecord/cdi_proquest_miscellaneous_2312553528) contesta o ranking |
| Multicompartimental | dendritos com NMDA e picos de cálcio | mais caro; sem benchmark encontrado |

Decisão: começar pelo **AdEx de ponto** e adicionar os dendritos humanos num contrato seguinte.

## Aprendizado

- **Regras locais e online**: STDP, BCM, homeostase e a forma neuromodulada de três fatores, em que um sinal como a dopamina decide se a marca deixada por um par pré e pós vira mudança de peso ([Frémaux & Gerstner](https://www.frontiersin.org/articles/10.3389/fncir.2015.00085/full), [Watt & Desai](https://www.frontiersin.org/journals/synaptic-neuroscience/articles/10.3389/fnsyn.2010.00005/full)). A homeostase age em horas ou dias, bem mais devagar que o STDP.
- **Gradiente substituto** (retropropagação adaptada a disparos): mais maduro para linguagem, com um modelo de 0,9 bi de parâmetros treinado do zero ([NeuronSpark](https://arxiv.org/abs/2603.16148)), mas é backprop.
- Decisão: **regras locais**, fiéis ao cérebro. O gradiente substituto fica como alternativa futura para linguagem.

## Esquecimento catastrófico

A teoria dos sistemas complementares: o **hipocampo** aprende rápido com códigos esparsos, o **neocórtex** extrai a estrutura devagar, e a **repetição (replay) durante o sono** transfere um para o outro ([Kumaran, Hassabis & McClelland, 2016](https://psych205.stanford.edu/papers/KumaranHassabisMcC16CLSUpdate.pdf)). É o caminho previsto para a lacuna que o projeto já tinha em aberto.

## Escala

- O cérebro tem [86 bilhões de neurônios](https://www.brainfacts.org/in-the-lab/meet-the-researcher/2018/how-many-neurons-are-in-the-brain-120418) e entre 10¹⁴ e 10¹⁵ sinapses, com ~7.000 por neurônio cortical ([AI Impacts](https://aiimpacts.org/scale-of-the-human-brain/)).
- Um benchmark de 2023 com o simulador [GeNN](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC9950635/) rodou cerca de 250 mil neurônios simples numa GPU de baixo custo, uns 340 mil vezes menos que o cérebro. Com dendritos detalhados o número cai bastante; teremos que medir na RTX 3060 desta máquina.

## Aproveitado dos rascunhos retirados

Os rascunhos `memoria-eficiente` e `escala-gpu` foram retirados, porque falavam de "milhões de parâmetros" de um MLP. Ficam como ideias para quando o estado for por neurônio e sinapse:

- CUDA como backend opcional, sem que o núcleo o conheça, preservando o build só com C++20 e CMake.
- O driver desta máquina (595.99) suporta CUDA 13.2, mas o CUDA Toolkit (`nvcc`) não está instalado.
- Evitar cópias extras do estado ao carregar, salvar e treinar; a GPU tem 12 GB e a RAM, 15 GiB.
