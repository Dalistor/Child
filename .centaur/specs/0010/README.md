# 2026 Homeostase e inibição: a taxa de disparo volta ao alvo e a população não explode

**Data:** 2026-11-02
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** cerebro@2
**Regras:** HOM-01, INB-01
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** master/0008, master/0009
**Solicitação original:** "Pode fazer o push e criar a spec" (próximo passo proposto: ligação sináptica, HOM-01 e INB-01)

## Objetivo e demonstração
Antes: neurônio, sinapse e dopamina funcionam isolados. Nenhuma sinapse entrega corrente a um neurônio, e nada impede a taxa de disparo de subir ou cair sem limite. Depois:
- **Transmissão:** um disparo pré-sináptico abre uma condutância no neurônio de destino, excitatória ou inibitória, que decai com o tempo.
- **Homeostase (HOM-01):** a escala sináptica multiplica os pesos de entrada de um neurônio até a taxa média voltar a 5 Hz.
- **Inibição (INB-01):** um neurônio inibitório genérico, em `src/interneuron/`, recebe a atividade de uma população excitatória e a contém.

Demonstração: no `ctest`, três casos novos imprimem:
- `synaptic transmission`: o potencial sobe depois de um disparo excitatório e desce depois de um inibitório;
- `homeostatic scaling`: a taxa em janelas de 20 s antes e depois de a entrada dobrar, com e sem homeostase;
- `inhibition`: a taxa da população ao longo do tempo, com e sem o neurônio inibitório.

Fora desta entrega:
- **APR-01**, que usa tudo isto e vem na próxima spec.
- **Plasticidade inibitória** (Vogels 2011): o INB-01 pede sinapse inibitória fixa nesta versão.
- **Tipos de interneurônio** (PV, SST, VIP).
- **CFG-01** (arquivo de parâmetros) e **ENG-02** (estado no engrama).
- **Estrutura de rede em `src/`.** A ligação entre neurônios fica nos testes, como na master/0009. Quando o APR-01 ou o tálamo precisarem dela, ela ganha pasta própria.

## Impacto previsto
- **Novos:** `src/interneuron/interneuron.{hpp,cpp}`, com os padrões do neurônio inibitório genérico e da sinapse inibitória fixa.
- **Alterados:**
  - `src/neuron/neuron.{hpp,cpp}`: condutâncias excitatória e inibitória que somam corrente. O `step(corrente)` atual continua, e o caso `adex neuron` segue sem edição.
  - `src/synapse/`: a escala homeostática, como diz a [arquitetura](../../system/architecture.md) ("peso, plasticidade STDP e escala homeostática").
  - `docs/parametros.md`: toda constante padrão nova (τ_E, τ_I, E_E, E_I, condutâncias, τ_hom, taxa-alvo) com faixa, unidade e fonte, para o PAR-01.
  - `CMakeLists.txt` e `tests/tests.cpp`.
- **Dependências:**
  - `neuron/` não conhece `synapse/`; recebe condutâncias como números.
  - `synapse/` não conhece `neuron/`; a escala recebe a taxa estimada como número.
  - `interneuron/` pode usar `neuron/` (é um neurônio com outros padrões). Nada do núcleo conhece `interneuron/`.
  - Os testes juntam as peças. O gerador de disparos de fundo usa o `Rng` determinístico existente, nunca `<random>`.
- **Compatibilidade:** `adex neuron`, `stdp synapse` e `three-factor plasticity` passam sem edição, e o hash do `reference weights` não muda. O `brain_layout` passa a verificar `src/interneuron/`.
- **PAR-01:** ganha os parâmetros desta entrega e continua parcial até o CFG-01.

## Autonomia e decisões
- **Autonomia do `cerebro@2`:** discretização, passo de tempo e parâmetros iniciais dentro de faixas publicadas, sem mudar o comportamento observável.
- **Escolhas técnicas desta entrega** (dentro da autonomia; registrar na diferença plano × resultado):
  - **Transmissão por condutância** com decaimento exponencial: `I_syn = g_E (E_E − V) + g_I (E_I − V)`. Valores de Vogels 2011: E_E = 0 mV, E_I = −80 mV, τ_E = 5 ms, τ_I = 10 ms. A corrente direta seria mais simples, mas a condutância faz a inibição puxar o potencial para E_I, sem passar dele. Isso é o "reduz o potencial do neurônio alvo" do INB-01, e é o modelo das fontes de [parametros.md §5](../../../docs/parametros.md).
  - **Peso → condutância:** o peso da sinapse continua em [0, w_max] e vira condutância por um fator g_max em nS. O valor fica dentro das faixas de Song 2000 e Vogels 2011, e o executor escolhe um que faça um neurônio disparar a ~5 Hz com a entrada do teste.
  - **Escala homeostática:**
    - um estimador de taxa por neurônio, um filtro exponencial dos disparos com τ_hom = 5 s;
    - todos os pesos de entrada são multiplicados pelo mesmo fator, que sobe quando a taxa estimada está abaixo do alvo e desce quando está acima. Isso preserva a proporção entre eles.
    - O executor escolhe a forma exata (por exemplo `w ← w · (1 + η (r_alvo − r) dt)`) e a taxa de aprendizado.
    - O corte em w_max pode quebrar a proporção. O teste usa pesos longe do teto, e a escolha fica registrada.
  - **Neurônio inibitório genérico:** um AdEx com padrões de disparo rápido, sem adaptação (b = 0, como o "tônico" de Naud 2008) ou os padrões da rede de Vogels 2011. Sinapse inibitória sem plasticidade.
  - **"Estabiliza" e "satura" no INB-01:** o contrato não dá números. Esta spec os mede assim, na média da população em janelas de 1 s:
    - *estabiliza*: nas últimas janelas, a taxa varia menos de 20% entre janelas vizinhas e fica abaixo de 60 Hz, o limite de instabilidade de Zenke 2013;
    - *satura*: sem inibição, a taxa cresce e passa de 60 Hz, ou chega perto do teto que o refratário e a adaptação permitem.
    Se o executor precisar de outra medida, ela fica registrada e não pode ser mais frouxa que esta.
- Nenhuma decisão humana aberta.

## Contexto técnico
- **Fontes:**
  - [cerebro v002](../../contracts/cerebro/v002.json) (HOM-01, INB-01, PAR-01).
  - [parametros.md §1, §4 e §5](../../../docs/parametros.md) e a [proposta de limiares](../../../docs/parametros.md#proposta-de-limiares) (5 Hz, ±20%, janelas de 20 s, τ_hom de 5 s).
  - [neuron.hpp](../../../src/neuron/neuron.hpp), [synapse.hpp](../../../src/synapse/synapse.hpp) e [rng.hpp](../../../src/model/rng.hpp).
  - Os casos `adex neuron`, `stdp synapse` e `three-factor plasticity` em [tests.cpp](../../../tests/tests.cpp).
  - [master/0008](../0008/README.md) e [master/0009](../0009/README.md).
- **Proteção existente:**
  - Os três casos do núcleo protegem a dinâmica atual.
  - Nada protege transmissão, homeostase nem inibição.
- **Custo dos testes:** o HOM-01 simula ~100 s a 0,1 ms (cerca de 10⁶ passos). O `ctest` inteiro deve continuar em poucos segundos com `-O3`. Se não ficar, registrar o tempo e reduzir o número de entradas, nunca os limiares do contrato.
- **Regras do projeto:** float32, determinismo, sem `<random>`, sem avisos com `-Wall -Wextra -Wpedantic`, identificadores em inglês.
- **Cada executor:**
  - carrega `clean-code` e lê `references/architecture.md` (neurônio, sinapse e interneurônio no núcleo, sem E/S);
  - confirma as fontes atuais;
  - usa Graphify só se necessário.

## Tasks
As três tasks editam `tests/tests.cpp` e `CMakeLists.txt`, e as duas últimas usam a transmissão da primeira, por isso rodam em série.

### Task 01 — Transmissão sináptica por condutância
**Regras:** — (base para HOM-01 e INB-01; PAR-01 parcial)
**Modo:** TDD
**Responsável:** a atribuir
**Arquivos:** `src/neuron/neuron.{hpp,cpp}`, `docs/parametros.md` (τ_E, τ_I, E_E, E_I, g_max) e `tests/tests.cpp` (caso `synaptic transmission` e um gerador de disparos de fundo com `Rng`)
**Depende de:** cerebro/NEU-01 e SIN-01 (integradas)
**Verificação:**
- **Risco vital:** um sinal ou unidade errada na corrente sináptica faz a inibição excitar, ou faz a homeostase compensar um erro em vez de uma mudança de entrada.
- **Lacuna:** nenhuma proteção.
- **O caso `synaptic transmission`:**
  - um disparo excitatório faz V subir e voltar ao repouso;
  - um disparo inibitório faz V descer, sem passar de E_I;
  - sem disparos, V fica no repouso;
  - o gerador de fundo com a mesma semente produz a mesma sequência.
- `adex neuron` sem edição. Ciclo vermelho → verde.

**Instrução para o executor:**
> Spec master/0010 — Task 01: dar ao neurônio AdEx (`cerebro@2`) entradas de condutância.
> - **Neurônio:** condutâncias excitatória e inibitória que recebem incrementos (em nS) e decaem com τ_E = 5 ms e τ_I = 10 ms. Elas somam `g_E (E_E − V) + g_I (E_I − V)` à corrente, com E_E = 0 mV e E_I = −80 mV (Vogels 2011). Manter `step(corrente)` e o comportamento do NEU-01.
> - **Independência:** o neurônio não conhece a sinapse e recebe números.
> - **Gerador de fundo:** nos testes, um gerador de disparos de Poisson por entrada, com o `Rng` existente. Mesma semente, mesma sequência.
> - **Parâmetros:** registrar as constantes novas em `docs/parametros.md` com faixa, unidade e fonte.
> - **Teste antes:** escrever `synaptic transmission`, vê-lo falhar e implementar.
> - **Limites:** float32, sem `<random>`, sem avisos; não tocar no MLP, no CLI, na sinapse nem na dopamina.
> - **Relatório:** fontes com linhas, valores escolhidos e a saída do teste. Não escrever estado, specs nem índices.

### Task 02 — Escala sináptica homeostática
**Regras:** HOM-01 (e PAR-01 parcial)
**Modo:** TDD
**Responsável:** a atribuir
**Arquivos:** `src/synapse/` (a escala; pode ser um arquivo além do principal), `CMakeLists.txt` (se houver arquivo novo), `docs/parametros.md` (τ_hom, taxa-alvo, taxa de aprendizado) e `tests/tests.cpp` (caso `homeostatic scaling`)
**Depende de:** Task 01
**Verificação:**
- **Risco vital:** sem a homeostase, o APR-01 aprende com a taxa errada ou silencia. Uma escala aditiva, ou que não preserva a proporção, apaga o que o STDP aprendeu.
- **Lacuna:** nenhuma proteção.
- **O caso `homeostatic scaling`** cobre os quatro aceites do HOM-01, num neurônio com várias entradas de fundo:
  - a entrada dobra, e a taxa média em janelas de 20 s volta a 4–6 Hz em até 50 s;
  - sem a homeostase, a mesma mudança deixa a taxa fora de 4–6 Hz;
  - a razão entre dois pesos de entrada é a mesma antes e depois da escala, até o arredondamento de float32;
  - o τ_hom padrão é 5 s.
- O teste imprime as taxas por janela. Ciclo vermelho → verde.

**Instrução para o executor:**
> Spec master/0010 — Task 02: implementar o HOM-01 do `cerebro@2` em `src/synapse/`.
> - **Escala:** um estimador de taxa (filtro exponencial dos disparos do neurônio, τ_hom = 5 s) e uma escala multiplicativa de todos os pesos de entrada em direção à taxa-alvo de 5 Hz. A forma exata e a taxa de aprendizado são escolha sua, dentro das faixas de `docs/parametros.md` §4.
> - **Independência:** a escala recebe a taxa e os pesos como números; não conhece o neurônio.
> - **Teste antes:**
>   - Em `tests/tests.cpp`, escrever `homeostatic scaling`: um neurônio com entradas de fundo (Task 01) chega a ~5 Hz, a taxa das entradas dobra, e as janelas de 20 s são medidas com e sem homeostase.
>   - Conferir também a razão entre pesos.
>   - Ver o teste falhar e implementar.
> - **Limites:** os da Task 01. O STDP pode ficar desligado neste caso, porque o aceite não o exige.
> - **Ao terminar:** rodar `ctest` e registrar a evidência do HOM-01 com `record-evidence.py`.
> - **Relatório:** fontes com linhas, evidência, taxas impressas, escolhas e tempo do teste.

### Task 03 — Neurônio inibitório genérico em `src/interneuron/`
**Regras:** INB-01 (e PAR-01 parcial)
**Modo:** TDD
**Responsável:** a atribuir
**Arquivos:** `src/interneuron/interneuron.{hpp,cpp}` (novos), `CMakeLists.txt` (uma linha em `child_core`), `docs/parametros.md` (padrões do interneurônio e peso inibitório) e `tests/tests.cpp` (caso `inhibition`)
**Depende de:** Task 01
**Verificação:**
- **Risco vital:** sem inibição, a rede com STDP explode e o degrau 0 não fecha. Uma sinapse inibitória que aprende pelo STDP excitatório desfaz o freio.
- **Lacuna:** nenhuma proteção.
- **O caso `inhibition`** cobre os dois aceites do INB-01, numa população excitatória pequena com sinapses recorrentes e STDP sob entrada forte constante:
  - com o neurônio inibitório, a taxa média estabiliza (critério em "Autonomia e decisões");
  - sem ele, a taxa cresce até saturar;
  - um disparo inibitório reduz o potencial do alvo;
  - o peso inibitório não muda durante a simulação.
- O teste imprime as taxas por janela. Ciclo vermelho → verde.

**Instrução para o executor:**
> Spec master/0010 — Task 03: implementar o INB-01 do `cerebro@2` em `src/interneuron/` (arquivo principal `interneuron`).
> - **Neurônio inibitório genérico:** um AdEx com padrões de disparo rápido (sem adaptação), mais a sinapse inibitória fixa que alimenta g_I no alvo. Sem tipos PV, SST ou VIP.
> - **Dependências:** `interneuron/` pode usar `neuron/`, e nada do núcleo conhece `interneuron/`.
> - **Teste antes:**
>   - Em `tests/tests.cpp`, escrever `inhibition`: uma população excitatória pequena (por exemplo 20–50 neurônios) com sinapses recorrentes e STDP, sob entrada forte constante.
>   - Medir a taxa média em janelas de 1 s, com e sem o neurônio inibitório, usando os critérios da spec.
>   - Ver o teste falhar e implementar.
> - **Limites:** os da Task 01. A ligação da população fica no teste.
> - **Ao terminar:** rodar `ctest` e registrar a evidência do INB-01.
> - **Relatório:** fontes com linhas, evidência, taxas impressas, escolhas e tempo do teste.

## Validação e integração
- **Critérios da entrega:**
  - HOM-01 e INB-01 implementados, com evidência corrente;
  - `ctest` verde, com os casos do núcleo inalterados, o `brain_layout` verificando `interneuron/` e o hash do `reference weights` inalterado;
  - build do zero sem avisos;
  - `docs/parametros.md` com todas as constantes novas.
- **Gates:** `validate-lifecycle.py . --ready cerebro/HOM-01` e `--ready cerebro/INB-01`.
- **Consolidação pelo coordenador:**
  - `.centaur/state/cerebro.json` (HOM-01, INB-01, PAR-01);
  - refazer as evidências que dependem dos arquivos alterados;
  - registro em `.centaur/implements/`;
  - linha "Onde estamos" do README, `AGENTS.md` e `architecture.md`.
- **Integração:** branch e PR, quando autorizado.

## Diferença entre plano e resultado
- **Implementação:** [0009](../../implements/0009/README.md).
- **O que seguiu o plano:**
  - condutâncias de Vogels 2011;
  - τ_hom de 5 s e alvo de 5 Hz;
  - interneurônio "a tônico" de Naud 2008 com sinapse fixa de 3 nS;
  - ligação da população nos testes.
- **Desvio técnico (dentro da autonomia):** a escala homeostática virou um ganho por neurônio (peso efetivo = peso × ganho) em vez de reescrever cada peso. Reescrever acumulava 4,7·10⁻⁴ de desvio de proporção em float32; o ganho preserva a proporção exatamente e é O(1) por passo. A `Synapse` ficou intocada.
- **Valores escolhidos:**
  - homeostase: η = 10⁻⁴/ms, entrada de fundo a 20 Hz e 1 nS de pico;
  - população: E→E com 10% e 12 nS, E→I com 50% e 1 nS, I→E denso com 3 nS;
  - os interneurônios não recebem fundo.
  Tudo está registrado em `parametros.md`. O η não tem valor publicado na mesma forma.
- **Critério de "estabiliza":** aplicado como planejado, mais a ausência de tendência entre os dois últimos blocos de 10 s.
- **Impacto real:** igual ao previsto, mais `src/synapse/homeostasis.*` como arquivo separado. As evidências afetadas foram refeitas.
- **Integração:** na branch `homeostasis-inhibition`; falta o PR e o merge.

## Checklist de conclusão
- [x] Task 01 — Transmissão sináptica por condutância
- [x] Task 02 — Escala sináptica homeostática
- [x] Task 03 — Neurônio inibitório genérico em `src/interneuron/`

## Execução
/centaur-driven-run master/0010

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
