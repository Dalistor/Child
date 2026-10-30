# Síntese: o que aprendemos até aqui

Consolidação de 2026-10-30 das duas rodadas de pesquisa. É a porta de entrada; os detalhes e todas as fontes estão em [pesquisa-cerebro.md](pesquisa-cerebro.md) e [ideias-cerebro.md](ideias-cerebro.md). Tudo se baseia em resumos de busca, não em artigos lidos por inteiro.

## Em uma frase

O Child é uma **base aberta de um cérebro simplificado**. Seus neurônios disparam e suas sinapses aprendem por regras locais, moduladas por um sinal de surpresa. As memórias se consolidam por repetição. A base é reproduzível, ramificável e tem peças trocáveis, para que outras pessoas a clonem e treinem.

## Sete lições

1. **O que torna um neurônio humano é o dendrito, não o disparo.** Sem as condutâncias NMDA, um neurônio piramidal detalhado é imitado por uma unidade simples. Com picos de cálcio dendríticos, um único neurônio humano resolve XOR. Daí a fidelidade em degraus: AdEx de ponto, depois segmentos dendríticos (estilo HTM) e por fim NMDA e cálcio.

2. **Aprender só com regras locais é possível, mas o STDP sozinho não basta.** Faltam duas coisas:
   - estabilidade, dada pela homeostase (escala sináptica, limiar BCM) e pela inibição (interneurônios PV, SST e VIP), sem a qual a excitação dispara sem limite;
   - um terceiro fator que diga *o que importa*, porque o STDP puro rende pouco sem um erro global.

3. **A predição é o fio que une tudo.** Prever o próximo byte é o objetivo. O erro dessa previsão (a surpresa) é a dopamina que libera a plasticidade. Os segmentos dendríticos são o mecanismo que prevê, e a queda do erro (o progresso) é o que move a curiosidade. Um único princípio dá ao projeto o objetivo, o sinal de aprendizado e a motivação.

4. **Cada estrutura do cérebro resolve um problema diferente com a mesma matéria-prima:**

   | Estrutura | Problema que resolve | Regra |
   |---|---|---|
   | Neocórtex | Extrair estrutura devagar e prever | STDP + dopamina, segmentos dendríticos |
   | Hipocampo | Guardar rápido sem misturar e completar a partir de pistas | Expansão esparsa (giro denteado) + atrator (CA3) |
   | Cerebelo | Mapear entrada em saída correta | Expansão esparsa + leitura com erro local (regra delta) |
   | Gânglios da base | Escolher entre ações concorrentes | Go/NoGo reforçado por dopamina |
   | Interneurônios | Manter o equilíbrio e abrir janelas de plasticidade | Inibição de retroalimentação e alimentação direta |

5. **O esquecimento catastrófico se resolve com dois sistemas e sono.** O hipocampo aprende rápido e o neocórtex aprende devagar. A repetição durante o sono transfere de um para o outro e, em modelos, protege memórias antigas. Se essa é a causa real no cérebro ainda é debatido.

6. **A esparsidade é recurso, não detalhe.** Só ~5% dos grânulos do giro denteado ficam ativos. As SDR quase não dão falso positivo, e o BDH usa ativações esparsas e positivas. A esparsidade dá capacidade, separação e economia de cálculo ao mesmo tempo.

7. **A escala é modesta e tem que ser medida.** O cérebro tem 86 bilhões de neurônios. Uma GPU de consumo roda centenas de milhares de neurônios simples. A simulação deve ser por eventos, determinística (desempate fixo e, possivelmente, ponto fixo) e guardar o estado dinâmico no engrama, não só os pesos.

## Modelo de referência

```
bytes ─► thalamus ─► córtex (neuron + synapse + interneuron, segmentos dendríticos)
                          │                           ▲
                          ▼                           │ replay no sono
                     cerebellum ─► próximo byte   hippocampus (esparso + CA3)
                          │
                 erro de predição
                          ▼
                      dopamine ─► libera a plasticidade das sinapses
                                       │
                                  engram (checkpoint, fork, linhagem)
```

## O que é base e o que é extensão

Como o Child é uma base para outros treinarem, o critério é: entra na base o que todo clone precisa para treinar com segurança e o que a diferencia do MLP.

| Base | Extensão (terceiros podem construir) |
|---|---|
| Neurônio AdEx, sinapse com STDP, dopamina como surpresa | Gânglios da base para gerar texto |
| Homeostase e inibição | Curiosidade e currículo por progresso |
| Segmentos dendríticos | e-prop e outras regras "quase locais" |
| Leitura cerebelar do próximo byte | Neuromoduladores além da dopamina |
| Hipocampo com repetição e `sleep` | Astrócitos, mapa cognitivo (TEM) |
| Determinismo, engrama versionado, `fork`, testes de comportamento | Backend CUDA |

A qualidade do texto gerado não é critério. Os critérios medem mecanismos: aprende o padrão, resolve XOR, diferencia contexto e não esquece.

## Roteiro consolidado

| Degrau | O que entrega | Como saber que funciona |
|---|---|---|
| 0 | `cerebro` v001 com homeostase e inibição | APR-01 se mantém em treino longo, sem a taxa de disparo divergir |
| 1 | Dopamina como surpresa | Acompanha a perda do MLP (o oráculo) no mesmo texto |
| 2 | Leitura cerebelar | Prevê o próximo byte acima do acaso numa fonte repetitiva |
| 3 | Segmentos dendríticos | Um neurônio resolve XOR; `abcabd` separa os contextos de `c` e `d` |
| 4 | Hipocampo e `sleep` | Aprender B não estraga A com sono; sem sono, estraga |
| 5 | Escala e engrama de estado | Neurônios/s medidos na RTX 3060; `fork` preserva o estado dinâmico |

## Decisões

**Tomadas:**
- AdEx primeiro.
- Só regras locais, sem retropropagação.
- Pastas com nomes de regiões do cérebro.
- O MLP fica como oráculo.
- CUDA é opcional.
- O Child é uma base.
- A pesquisa fica em `docs/`.

**Em aberto:**
- A homeostase e a inibição entram numa nova versão do `cerebro` ou num contrato próprio?
- O sinal de cima do e-prop conta como local?
- Ponto fixo ou ponto flutuante?
- Que estrutura usar para a fila de eventos? A resposta depende de medir.

## Grau de confiança

- **Firme:** dopamina como erro de predição; divisão giro denteado/CA3; o STDP precisa de estabilizador; teoria dos sistemas complementares; Marr-Albus para o cerebelo; AdEx prevê bem o tempo dos disparos.
- **Moderado:** papéis de PV, SST e VIP; sono contra o esquecimento; segmentos dendríticos como modelo de neurônio; critérios de janela crítica.
- **Fraco ou não verificado:** papéis de serotonina, noradrenalina e acetilcolina; astrócitos como memória; os números do BDH; qual fila de eventos é melhor em CPU.
