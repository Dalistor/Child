# [0009] Homeostase e inibição

**Data:** 2026-11-02
**Status:** Concluído
**Modo:** TDD
**Spec:** `.centaur/specs/0010/` — Tasks 01, 02 e 03

## Solicitação
"Pode fazer o push e criar a spec" e "Sim" (executar).

## O que foi feito
- **Task 01 — transmissão:**
  - `src/neuron/` ganhou receptores: `excite(nS)` e `inhibit(nS)` abrem condutâncias que decaem com τ_E 5 ms e τ_I 10 ms.
  - A corrente sináptica é `g_E (E_E − V) + g_I (E_I − V)`, com E_E 0 mV e E_I −80 mV (Vogels 2011).
  - O `step(corrente)` continua igual quando não há condutância.
- **Task 02 — HOM-01:**
  - `src/synapse/homeostasis.{hpp,cpp}`: um sensor de taxa por neurônio (filtro com τ_hom 5 s) e um ganho que multiplica todos os pesos de entrada, com alvo de 5 Hz.
- **Task 03 — INB-01:**
  - `src/interneuron/`: neurônio inibitório genérico, um AdEx de disparo rápido sem adaptação (conjunto "a tônico" de Naud 2008).
  - A sinapse inibitória é fixa, com 3 nS por disparo.
- **Testes:** casos `synaptic transmission`, `homeostatic scaling` e `inhibition`, mais o gerador `SpikeSource` de disparos de Poisson com o `Rng` do projeto.
- **Documentos:** `docs/parametros.md` registra os valores do código e a configuração dos testes. README, `AGENTS.md` e `architecture.md` atualizados.

## Arquivos criados
- `src/synapse/homeostasis.{hpp,cpp}`, `src/interneuron/interneuron.{hpp,cpp}`

## Arquivos modificados
- `src/neuron/neuron.{hpp,cpp}`, `CMakeLists.txt`, `tests/tests.cpp`, `docs/parametros.md`, `README.md`, `AGENTS.md`, `.centaur/system/architecture.md`

## Decisões técnicas
- **Ganho por neurônio em vez de reescrever cada peso:**
  - A spec previa multiplicar cada peso de entrada a cada passo. Em float32, isso acumulou 4,7·10⁻⁴ de desvio na proporção entre pesos ao longo de 60 s.
  - O ganho comum (peso efetivo = peso × ganho) é a mesma escala multiplicativa, mas preserva a proporção exatamente (desvio de 1,2·10⁻⁷) e custa uma multiplicação por passo, em vez de uma por sinapse.
  - Os pesos que o STDP altera ficam intactos.
- **η da homeostase = 10⁻⁴/ms (1/η = 10 s):** não há valor publicado na mesma forma. A escala de tempo fica abaixo do τ_crit de Zenke 2013.
- **Entrada de fundo a 20 Hz por entrada:** a 10 Hz, mesmo com peso máximo e 1 nS, o neurônio só chegava a ~1,3 Hz, e o alvo ficava inalcançável. Song 2000 usa 10–40 Hz.
- **Configuração da população no teste do INB-01:**
  - E→E com 10% e 12 nS de pico; I→E denso; E→I com 50%.
  - Os interneurônios não recebem fundo e respondem só à população.
  - Numa rede de 200 neurônios, sinapses mais fortes compensam o número pequeno de entradas. Isso está registrado em `parametros.md`.
  - Os valores foram escolhidos num protótipo, porque com I→E esparso a inibição freava sem estabilizar.
- **Limites dos testes ajustados à física do modelo:**
  - A adaptação subliminar do AdEx dá um leve rebote depois de cada potencial pós-sináptico, por isso o teste não exige que V nunca cruze o repouso.
  - Um potencial inibitório de 3 nS num neurônio com g_L de 30 nS tem ~0,34 mV, e o limite do teste é 0,2 mV.

## Como validar
`ctest --test-dir build`: os três casos novos imprimem as taxas por janela, com e sem homeostase e com e sem inibição.

## Contrato e realização
- `cerebro@2` HOM-01 (ev-7042385d5ee440af9528d334a339c44b) e INB-01 (ev-a31dfb97b0db4dc6909870a382297383) implementadas e verificadas.
- PAR-01 ganhou receptores, homeostase e interneurônio; continua parcial.
- As evidências afetadas foram refeitas.
- Entrega local, na branch `homeostasis-inhibition`.

## Resultado da validação
Ciclo vermelho → verde em cada task. `ctest` 3/3 em ~5 s, build do zero sem avisos.
- **HOM-01:** antes, 4,95 Hz. Com a entrada dobrada, o neurônio passa por 10,4 / 1,7 / 6,5 / 4,8 Hz por 10 s, e a janela de 30 a 50 s fica em 4,95 Hz. Sem homeostase, 41,6 Hz.
- **INB-01:** com inibição, 8–9 Hz em todas as janelas de 1 s. Sem inibição, 52 → 27 → 120–158 Hz.
- `adex neuron`, `stdp synapse`, `three-factor plasticity` e `reference weights` não mudaram.
