# Arquitetura

O Child é uma **versão bem simplificada do cérebro**, em C++20 e sem dependências externas. Este documento descreve o que existe hoje e explica as decisões. As regras normativas ficam nos contratos:
- [`cerebro`](../contracts/cerebro/v002.json): neurônio, sinapse, dopamina, homeostase, inibição e parâmetros;
- [`encefalo`](../contracts/encefalo/v001.json): o cérebro integrado e o CLI;
- [`dendritos`](../contracts/dendritos/v001.json), [`hipocampo`](../contracts/hipocampo/v001.json) e [`neurogenese`](../contracts/neurogenese/v001.json);
- [`engrama`](../contracts/engrama/v001.json), [`reprodutibilidade`](../contracts/reprodutibilidade/v001.json) e [`base-aberta`](../contracts/base-aberta/v001.json);
- [`base`](../contracts/base/v002.json): o MLP, mantido como oráculo.

A pesquisa está em [docs/](../../docs/README.md) e o roteiro, na [síntese](../../docs/sintese.md). Atualizado em 2026-11-07.

## Fluxo de um byte

```
fonte (arquivo | pasta | stdin) ─ capture/ ─► bytes
                                                │
                        thalamus/  canais relé de Poisson: o código esparso do byte dispara
                                                │ disparos (passo de 0,1 ms, byte_ms = 10 ms)
                                                ▼
   cortex/   células piramidais AdEx ◄─ sinapses STDP × dopamina (três fatores), escala homeostática
             segmentos distais (platô NMDA) ◄─ células ativas no byte anterior
             interneurônios genéricos ─► inibição de retroalimentação
                                                │ contagem de disparos por célula, traço (τ 30 ms)
                                                ▼
   cerebellum/  fibras musgosas (canais relé + células corticais) ─► grânulos (4 entradas, 5% ativos)
                ─► unidades de Purkinje ─► P(próximo byte); regra delta local pelo erro (fibra trepadeira)
                                                │
               surpresa = −ln P(byte que veio) ─┼─► dopamine/  libera a parte inesperada da surpresa
                                                ├─► hippocampus/  guarda o episódio (padrão, byte); sleep o repete
                                                └─► brain/neurogenesis  surpresa alta e parada ⇒ nascem células
                                                ▼
   engram/   seção "brain": parâmetros em texto + estrutura + estado dinâmico completo; linhagem e fork
```

`src/brain/brain.cpp` orquestra o fluxo: a cada byte, `Brain::perceive(byte, learn)`.

## Módulos e dependências

| Pasta | Estrutura | Responsabilidade | Depende de |
|---|---|---|---|
| `neuron/` | neurônio | AdEx com receptores de condutância; `dendrite`: pico de cálcio (dCaAP) e segmento distal | `model/rng.hpp` |
| `synapse/` | sinapse | STDP de pares, marca de elegibilidade, escala homeostática | nada |
| `dopamine/` | dopamina | Sinal global fásico, o terceiro fator | nada |
| `interneuron/` | interneurônio | Neurônio inibitório genérico (AdEx tônico) | `neuron/` |
| `thalamus/` | tálamo | Célula relé de Poisson por evento; código esparso de cada byte | `model/rng.hpp` |
| `cortex/` | neocórtex | População piramidal, sinapses tálamo-corticais, segmentos, inibição, crescimento | `neuron/`, `synapse/`, `interneuron/` |
| `cerebellum/` | cerebelo | Expansão em grânulos e leitura por regra delta | `model/rng.hpp` |
| `hippocampus/` | hipocampo | Episódios por amostragem de reservatório, repetição no sono, completação | `model/rng.hpp` |
| `brain/` | encéfalo | Liga tudo; parâmetros com faixa, política de neurogênese, serialização | todos acima |
| `engram/` | engrama | Formato em seções, checksum, escrita atômica, linhagem, `fork` | `model/model.hpp` (só `Config`) |
| `capture/` (legada) | — | Fonte → bytes | nada |
| `model/` (legada) | — | MLP oráculo e `Rng` | nada |
| `main.cpp` | — | CLI: orquestra captação, cérebro ou MLP e engrama | todos |

Regras de dependência:
- As peças do cérebro não conhecem o engrama nem a E/S.
- Cada peça expõe `template <class Archive> void persist(Archive&)`, que nomeia seus campos uma vez. `brain/archive.hpp` usa essa função para gravar e para ler.
- O engrama guarda a seção `brain` como um bloco opaco.
- Só o CLI conhece todas as camadas.

## Decisões

- **Passo fixo com fontes por evento.** Os neurônios integram em Euler a 0,1 ms. As células relé sorteiam o intervalo até o próximo disparo, um sorteio por disparo.
- **Tempos em `double`.** As sinapses usam `Millis`, para a janela do STDP continuar exata em treinos longos. Pesos e estados ficam em float32.
- **Três fatores com reforço por intervalo.** As propostas do STDP viram marcas de elegibilidade. A cada `reinforce_ms` (50 ms), a dopamina integrada no intervalo converte as marcas em peso: `dw = e · ∫D dt`.
- **A dopamina é a surpresa inesperada.** Ela vale `max(0, surpresa − surpresa esperada)`, com a esperada numa média móvel de 100 bytes, como o erro de predição de Schultz 1997.
- **O cerebelo lê o tálamo e o córtex.** As fibras musgosas carregam a entrada sensorial e a cortical. A regra delta só altera as sinapses dos grânulos ativos.
- **O hipocampo guarda uma amostra uniforme** de tudo o que foi vivido (reservatório). O sono repete os episódios embaralhados, intercalando o antigo e o novo.
- **A neurogênese segue erro e estagnação.** Por janela de bytes, se a surpresa média passa de `min_surprise` e não cai `min_improvement` em relação à janela anterior, nasce um lote de células, até o teto. As células novas são mais plásticas por `young_ms` (Ge 2007). O cerebelo ganha os grânulos que as leem.
- **Conectividade em contagens fixas.** Os interneurônios têm fan-in e fan-out fixos, não probabilidades, para que um córtex grande fique tão esparso quanto um pequeno.
- **Variáveis de ambiente controlam a execução sem reescrever o que está gravado:**
  - `CHILD_NEUROGENESIS` e `CHILD_MAX_NEURONS` valem em cada execução;
  - `CHILD_PARAMS` e `CHILD_ENGINE` só no `init`.

## Formato do engrama (versão 2, little-endian)

`"CHLD"` · versão `u32` · id/pai/passos `u64` · tamanho do nome `u32` + nome · nº de seções `u32` · seções (tipo `u32`, versão `u32`, tamanho `u64`, dados) · FNV-1a 64 bits de tudo o que precede.
- **Seção 1** (MLP): embed/context/hidden `u32`, nº de parâmetros `u64` e `float32[]`.
- **Seção 2** (cérebro, versão 1): semente `u64`, os parâmetros como texto `nome = valor`, e o estado de cada peça na ordem de `Brain::persist`. São gravados:
  - o `Rng`;
  - o tálamo (as fontes de cada canal);
  - o córtex (potenciais, adaptação, condutâncias, ganhos, sinapses com traços e marcas, segmentos, ligações e nascimento de cada célula);
  - o cerebelo (entradas dos grânulos, pesos e viés);
  - o hipocampo (episódios);
  - a dopamina, a neurogênese, os traços e a última previsão.

Tipo ou versão de seção desconhecidos são recusados, nomeando o tipo. A escrita usa `.tmp` seguido de `rename`. Um cérebro virgem padrão ocupa cerca de 11 MB, quase tudo pesos do cerebelo.

## Escala medida

`child bench --bytes 300 --neurons N`, numa thread de um Intel Core i5-10400F (2,9 GHz), build portátil `-O3`, medido em 2026-11-07:

| células piramidais | interneurônios | bytes/s | segundos simulados por segundo | passos de neurônio/s |
|---|---|---|---|---|
| 256 (padrão) | 64 | 538 | 5,4 | 1,7·10⁷ |
| 1024 | 256 | 121 | 1,2 | 1,5·10⁷ |
| 4096 | 1024 | 27 | 0,3 | 1,4·10⁷ |

A máquina tem uma RTX 3060, mas sem o toolkit CUDA, por isso o backend de GPU fica como extensão, como prevê a síntese. O núcleo não depende dele.

## Limites conhecidos

- **APR-01 depende da semente.** A configuração do teste passa em 5 das sementes 1 a 8 ([parametros.md §6](../../docs/parametros.md)).
- **Os segmentos dendríticos não mostram ganho medido no `abcabd`.** Sem eles, os dois contextos continuam separados (0,96 e 0,95 contra 0,96 e 0,96). A separação vem da história do próprio córtex (adaptação, condutâncias e inibição), que torna a representação de "b" diferente em cada contexto (cosseno de 0,02 entre contextos e 0,95 dentro do mesmo). Os segmentos estão ligados e aprendem, mas ainda falta um teste em que eles sejam necessários.
- **Vazão de ~500 bytes/s no tamanho padrão.** Ensinar os 100 KB de `src/` leva cerca de 3 minutos por época.
- **A previsão de texto é modesta.** O critério é o mecanismo, não a qualidade do texto. O MLP continua melhor como modelo de linguagem.
- **Em texto variado, a base cresce até o teto.** Ensinar os 102 KB de `src/` por uma época levou a base padrão de 256 a 1024 células (o teto) em 9 min, com perda média de 2,63 nats. A surpresa fica acima de 1,5 nat, e o crescimento ainda não mostrou ganho medido de previsão. Use `CHILD_MAX_NEURONS` ou `CHILD_NEUROGENESIS=off` para conter o crescimento.
- **O sono só consolida o cerebelo.** O córtex não é repetido no sono.
