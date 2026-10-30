# Ideias: o que o cérebro e as redes neurais oferecem ao Child

Segunda rodada de pesquisa, de 2026-10-30, que complementa [pesquisa-cerebro.md](pesquisa-cerebro.md) (neurônio AdEx, STDP, três fatores, CLS, escala). Aqui entra o que faltava: inibição, homeostase, predição, memória esparsa, estruturas além do hipocampo, desenvolvimento e engenharia da simulação.

**Limite da fonte:** tudo veio de resumos devolvidos por buscas, não dos artigos completos. Vários itens são preprints, e os papéis dos neuromoduladores e dos astrócitos são hipóteses. Nada aqui é decisão; é matéria-prima para contratos.

## 1. O que o cérebro faz e o projeto ainda não tem

### 1.1 Inibição e homeostase (pré-requisito do `cerebro`)

- STDP só com excitação diverge: o reforço depende de coincidência, então a atividade escala sem limite. Os estabilizadores clássicos são o limiar deslizante BCM, a escala sináptica e a plasticidade inibitória, e não são excludentes ([Nottingham](https://nottingham-repository.worktribe.com/OutputFile/848607), [Abbott & Nelson 2000](https://cenl.ucsd.edu/Jclub/Abbott-Nelson+taming-beast+NN+2000.pdf)). O BCM só garante estabilidade numa faixa de escalas de tempo da homeostase ([PMC5318375](https://pmc.ncbi.nlm.nih.gov/articles/PMC5318375)).
- Interneurônios: PV (inibição perissomática, feedforward), SST (inibição dendrítica, feedback) e VIP (inibe o SST, ou seja, desinibe o piramidal) ([PMC7176419](https://pmc.ncbi.nlm.nih.gov/articles/PMC7176419), [arXiv 2510.20848](https://arxiv.org/html/2510.20848v1)). A divisão de papéis ainda é debatida.
- **Ideia:** o critério APR-01 ("dispara mais ao padrão que ao ruído") provavelmente fica frágil sem um mecanismo de estabilidade. Vale incluir, já no `cerebro`, um alvo de taxa de disparo por neurônio (escala sináptica lenta) e um neurônio inibitório simples. Pasta sugerida: `src/interneuron/`.

### 1.2 Dendritos como segmentos (um caminho mais barato para o "neurônio humano")

- No HTM, cada célula tem vários segmentos dendríticos, cada um reconhecendo um contexto; só uma célula por coluna dispara dentro de uma sequência aprendida, o que representa a mesma entrada de modos diferentes conforme o contexto ([Numenta](https://discourse.numenta.org/t/how-does-the-htm-model-learn-about-sequences/2359), [Hawkins & Ahmad](https://alphaxiv.org/abs/1511.00083)). A regra é hebbiana local, com sinapses "potenciais" de permanência escalar.
- **Ideia:** o contrato "neurônio humano" (NMDA e picos de cálcio, critério XOR) pode ter uma versão intermediária: segmentos com limiar não linear, sem multicompartimentos. Para bytes, isso dá memória de sequência de ordem alta, exatamente o que a janela fixa de 16 bytes do MLP não tem.
- Representações esparsas distribuídas (SDR) têm baixa taxa de falso positivo ao reconhecer padrões por subamostragem ([arXiv 1503.07469](https://arxiv.org/pdf/1503.07469)).

### 1.3 Hipocampo em partes

- Giro denteado: expansão e esparsificação (cerca de 5% dos grânulos ativos), o que separa entradas parecidas. CA3: colaterais recorrentes que completam padrões, como um atrator ([PMC2976779](https://pmc.ncbi.nlm.nih.gov/articles/PMC2976779)). Ressalva: esparsidade é necessária mas não suficiente para separar bem.
- Redes de Hopfield modernas têm regra de atualização equivalente à atenção do transformer e armazenam exponencialmente mais padrões ([Ramsauer et al.](https://arxiv.org/abs/2008.02217)). A busca não encontrou nada sobre plausibilidade biológica.
- Modelo recente de consolidação: a repetição hipocampal treina uma rede generativa no neocórtex, e memórias consolidadas ficam mais sujeitas a distorção por esquema ([Spens & Burgess](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC10963272/)). Outro preprint defende repetição em espaço latente em vez de guardar exemplos ([arXiv 2507.11393](https://arxiv.org/pdf/2507.11393)).
- **Ideia:** `src/hippocampus/` com três partes: codificador esparso (k-vencedores, estilo giro denteado), memória autoassociativa (estilo CA3) e fila de repetição.
- **Ideia, fase de sono:** em modelos talamocorticais, fusos favorecem consolidar várias memórias de forma independente e oscilações lentas favorecem a mais forte; o sono simulado após aprender protegeu memórias antigas ([PMC6053241](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6053241/), [Bazhenov lab](https://www.bazhlab.ucsd.edu/?p=366)). No CLI, algo como `child sleep a.child`: desliga a entrada externa e reproduz o que o hipocampo guardou. A origem causal da repetição no cérebro real é debatida.
- Mapas cognitivos: o Tolman-Eichenbaum Machine propõe que o entorrinal guarda a estrutura e o hipocampo liga estrutura a sensação ([Whittington et al.](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC7707106/)). Para texto, a "estrutura" seria a gramática de transições entre bytes. Especulativo e caro; fica como horizonte.

### 1.4 Cerebelo: leitura supervisionada local

- Marr-Albus: grânulos expandem a entrada em código esparso; a fibra trepadeira entrega um sinal de erro que controla a plasticidade das sinapses fibra paralela → Purkinje ([J Physiol 2020](https://bicr.atr.jp/~kawato/Ppdf/J_Physiol_2020.pdf)). Um modelo de 2025 trata Purkinje e núcleos como classificador linear treinado por erro de trepadeira sobre características aleatórias esparsas ([PLOS CB](https://econpapers.repec.org/RePEc:plo:pcbi00:1014595)).
- **Ideia:** é o jeito honesto de ter uma **camada de saída que prevê o próximo byte** sem retropropagação. A regra delta de uma camada é local. Grânulos fixos e esparsos + leitura plástica supervisionada. Pasta: `src/cerebellum/`.

### 1.5 Gânglios da base: escolher o próximo byte

- Vias direta (Go) e indireta (NoGo) com aprendizado dopaminérgico nas sinapses córtex→estriado; o mapeamento para Q-learning/SARSA é comum ([PMC3462417](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3462417/)). Há debate se a divisão direta/indireta é necessária para selecionar ou só para aprender ([PMC4657096](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4657096/)).
- **Ideia:** amostrar o próximo byte como competição entre candidatos, com a dopamina reforçando escolhas que acertaram. Liga a dopamina do contrato à geração de texto.

### 1.6 Dopamina como erro de predição, e os outros moduladores

- A dopamina fásica acompanha o erro de predição de recompensa ([PMC3195531](https://pmc.ncbi.nlm.nih.gov/articles/PMC3195531)).
- **Ideia central para este projeto:** definir a dopamina como **a surpresa ao prever o próximo byte** (previu mal → sinal alto; previu bem → baixo). Isso dá ao terceiro fator uma fonte natural, sem recompensa externa, e a perda do MLP de hoje serve de oráculo para conferir se o sinal acompanha o erro.
- Hipóteses de Doya para os outros três (evidência fina): serotonina ↔ desconto temporal, noradrenalina ↔ amplitude da exploração, acetilcolina ↔ taxa de atualização da memória ([OIST](https://www.oist.jp/node/40341)). **Ideia:** expor como parâmetros globais do aprendizado (`dopamine/` e vizinhos), não como neurônios.

### 1.7 Predição como princípio

- Codificação preditiva em redes de disparos com plasticidade hebbiana local: [PC-SNN](https://arxiv.org/pdf/2211.15386), [SNN-PC](https://pmc.ncbi.nlm.nih.gov/articles/PMC11045951), erro calculado em compartimentos dendríticos ([Mikulasch et al.](https://ar5iv.labs.arxiv.org/html/2205.05303)). O Forward-Forward não apareceu nas buscas.
- **Ideia:** combina com 1.2 (segmentos dendríticos preveem a próxima entrada) e com 1.6 (erro = dopamina). Esta é a espinha dorsal mais coerente que encontrei para "prever o próximo byte" sem backprop.

### 1.8 O nome "Child": desenvolvimento

- Janelas críticas: modelos sugerem que a janela abre quando a inibição suprime a atividade espontânea, sem mudar a regra de plasticidade ([Toyoizumi et al. 2013](https://web.math.princeton.edu/~sswang/developmental-diaschisis-references/toyoizumi_hensch_miller13_neuron_critical-period-theory.pdf)). Sinapses "inatas" montam a rede básica e sinapses "gestalt" refinam com experiência (silenciosas que se desmascaram, estabilizam ou são podadas) ([PMC7380267](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC7380267/)).
- Curiosidade: motivar-se por **progresso de aprendizado** (queda do erro de predição), não por erro bruto, evita travar em ruído impossível de prever ([Oudeyer](https://www.pyoudeyer.com/?p=19), [Kaplan & Oudeyer](https://www.frontiersin.org/journals/neuroscience/articles/10.3389/neuro.01.1.1.017.2007/text)). Eles sugerem que a dopamina tônica pode sinalizar esse progresso.
- **Ideias:**
  - Base virgem = sinapses inatas (semente); `feed` = experiência. A **linhagem e o `fork` já são uma metáfora de hereditariedade**: o filho herda o engrama do pai e diverge.
  - Plasticidade com estágios: alta no início, com poda e estabilização depois (modo `--stage`), em vez de taxa de aprendizado constante.
  - Na captação, ordenar o currículo pela fonte onde o progresso é maior (`feed` escolhendo entre arquivos).

### 1.9 Astrócitos (horizonte, evidência fraca)

Um modelo de 2025 (MIT) trata cada processo astrocitário como unidade computacional, ligando mais de dois neurônios por sinapse tripartite e elevando a capacidade pela teoria de memória associativa densa ([IBM](https://research.ibm.com/blog/astrocytes-cognition-ai-architectures), [Neuroscience News](https://neurosciencenews.com/astrocytes-memory-capacity-29154/)). As teorias de neurônio-astrócito ainda estão no começo ([arXiv 2311.08135](https://arxiv.org/html/2311.08135v2)). Não recomendo agora.

## 2. O que as redes neurais atuais oferecem

| Trabalho | O que traz | Encaixe |
|---|---|---|
| [e-prop](https://link.springer.com/article/10.1038/s41467-020-17236-y) (Bellec et al., 2020) | Fatora o gradiente em traços de elegibilidade locais × sinal de aprendizado de cima; competitiva com BPTT no TIMIT, com neurônios adaptativos (ALIF). Versão por eventos no NEST: [arXiv 2511.21674](https://arxiv.org/html/2511.21674v1) | Estende o terceiro fator do contrato a redes recorrentes. O sinal de cima ainda precisa chegar aos neurônios, então é "quase local": vale decidir se cabe na regra "sem retropropagação" |
| [Dragon Hatchling (BDH)](https://arxiv.org/abs/2509.26507) | Rede escala-livre de partículas-neurônio; estado de contexto nas **sinapses** (hebbiano); ativações esparsas e positivas; relata paridade com GPT-2 de 10M a 1B | Valida "estado na sinapse". Resultados são da própria equipe e li só o resumo; confirme no artigo antes de usar |
| Texto → disparos | [Representação de texto com STDP](https://arxiv.org/pdf/2006.14894): 80,19% no 20 newsgroups (classificação, não geração) | Mostra que a codificação funciona; geração fica em aberto |
| [Sequências com STDP](https://arxiv.org/pdf/1911.10943) | Regra contínua compatível com STDP para modelos generativos de padrões espaço-temporais | Base para memória de sequência |
| STDP puro | Costuma render pior que backprop por falta de erro global ([arXiv 2207.02727](https://ar5iv.labs.arxiv.org/html/2207.02727)) | Argumento para o terceiro fator e para a leitura cerebelar |

Não achei um modelo de linguagem de bytes em disparos com STDP: o terreno é aberto, e vale tratar a primeira versão como experimento com critério medido contra o MLP.

## 3. Engenharia da simulação

- Integração exata por eventos para neurônios com correntes exponenciais e correntes de adaptação ([Brette](https://romainbrette.fr/exact-simulation-of-integrate-and-fire-models-with-exponential-currents/)). Atenção: a exatidão vale para I&F lineares; o AdEx tem a exponencial no potencial, então precisa de passo fixo ou aproximação.
- Segundo Morrison et al. (citado nos resultados, sem link direto), o atraso mínimo de propagação elimina a fila central de eventos. Um artigo de 2025 ([arXiv 2512.05906](https://arxiv.org/abs/2512.05906v1)) sugere que em CPU filas em árvore ou FIFO vão bem e anéis circulares vão melhor em GPU: **meça antes de escolher**.
- Determinismo (regra do projeto): ordem de processamento de eventos no mesmo instante precisa ser fixa (desempate por índice). Mesma semente, mesmos pesos.
- Loihi 2 usa disparos graduados (inteiros) e permite fatores modulatórios arbitrários calculados pelo neurônio pós ([arXiv 2111.03746](https://arxiv.org/pdf/2111.03746), [arXiv 2511.01553](https://arxiv.org/pdf/2511.01553)). **Ideia:** disparo com carga inteira abre espaço para pesos e traços em ponto fixo, o que facilita determinismo e GPU.
- Engrama: guardar o estado por neurônio (potencial, adaptação, traços) além dos pesos, senão o `fork` retoma "com amnésia de curto prazo".

## 4. Roteiro sugerido (cada degrau com critério medível)

| Ordem | Contrato | Critério de aceitação (rascunho) |
|---|---|---|
| 0 | `cerebro` v001 (aprovado) + homeostase e inibição simples | APR-01 continua valendo com 1.000 épocas, sem a taxa de disparo divergir |
| 1 | Predição e dopamina como surpresa | Dopamina acompanha a perda do MLP no mesmo texto (correlação mínima a definir) |
| 2 | Leitura cerebelar para bytes | Prevê o próximo byte acima do acaso (1/256) numa fonte repetitiva, sem retropropagação |
| 3 | Dendritos em segmentos | Um neurônio resolve XOR; sequência `abcabd` diferencia o contexto de `c` e `d` |
| 4 | Hipocampo (esparso + CA3 + repetição) e `sleep` | Após aprender B, a perda em A não piora além de um limite; sem sono, piora |
| 5 | Desenvolvimento e curiosidade | Currículo por progresso supera ordem aleatória numa fonte mista |
| 6 | Escala/GPU e engrama de estado | Medir neurônios/s na RTX 3060; `fork` preserva estado dinâmico |
| Horizonte | e-prop, gânglios da base, astrócitos, mapa cognitivo | Só após o degrau 4 |

## 5. Decisão do usuário: o Child é uma base

Decidido em 2026-10-30: o objetivo final do Child **é ser uma base**; outras pessoas clonam o repositório e treinam a partir dela. Não é um produto que gera texto bem. Consequências para as ideias acima:

- **Sobe de prioridade:** tudo que torna a base confiável e extensível.
  - Determinismo e reprodutibilidade (mesma semente, mesmos pesos, também entre máquinas; ponto fixo ajuda).
  - Formato de engrama estável e versionado, com `fork` e linhagem como o modo oficial de derivar o próprio cérebro a partir da base.
  - Peças pequenas e substituíveis por região (`neuron/`, `synapse/`, `dopamine/`, `hippocampus/`), com interfaces estreitas, para que alguém troque o AdEx por outro neurônio sem tocar no resto.
  - Parâmetros e regras de aprendizado configuráveis sem recompilar; `feed` aceitando qualquer fonte de bytes.
  - Homeostase e inibição (1.1): um clone que treina sem divergir vale mais que um que aprende rápido.
  - Testes de comportamento por degrau (XOR, sequência, esquecimento) como contrato de que a base funciona.
  - Documentação de "como treinar o seu": o que `init`, `fork`, `feed`, `sleep` e `info` fazem.
- **Desce para horizonte:** gânglios da base como gerador de texto (1.5), curiosidade e currículo (1.8), astrócitos (1.9), mapa cognitivo e e-prop. São extensões que terceiros podem construir sobre a base, não parte dela.
- **Fica no núcleo mínimo:** neurônio, sinapse com STDP, dopamina como surpresa, homeostase, depois dendritos em segmentos e hipocampo com repetição, que são o que dá à base algo que o MLP não faz.
- **Qualidade de texto deixa de ser critério.** Os critérios medem mecanismo (aprende o padrão, resolve XOR, não esquece), como já é nas exclusões do contrato `cerebro`.
- **Pendências de projeto aberto** (fora do escopo da pesquisa): licença, guia de contribuição e um exemplo de ponta a ponta no README.

## 6. Perguntas que ainda mudam o plano

1. O sinal de cima do e-prop conta como "sem retropropagação"? Como é extensão, pode ficar de fora da base.
2. A homeostase e a inibição entram no `cerebro` v001 (exige nova versão, pois está aprovado) ou viram contrato próprio? Pela decisão acima, recomendo entrar logo.
