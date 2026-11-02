# 2026 Neurônio AdEx que dispara e se adapta, e sinapse que aprende por STDP

**Data:** 2026-11-02
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** cerebro@2
**Regras:** NEU-01, SIN-01
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Crie a spec do neurônio AdEx e da sinapse"

## Objetivo e demonstração
Antes: não há neurônio no código; o `cerebro@2` só tem o REG-01 implementado. Depois:
- `src/neuron/` tem um neurônio AdEx de ponto que integra corrente, dispara, reinicia e se adapta.
- `src/synapse/` tem uma sinapse cujo peso muda pela ordem e pelo intervalo entre disparos pré e pós (STDP), sempre entre 0 e o máximo.
- Os dois são as primeiras peças do cérebro, sem retropropagação e sem dependência entre si.

Demonstração: `ctest --test-dir build` mostra os casos novos. Um mostra a sequência de disparos sob corrente constante, com o intervalo entre eles crescendo (adaptação). O outro mostra a curva de mudança de peso por intervalo pré→pós, positiva antes e negativa depois, e encolhendo com a distância.

Fora desta entrega: DOP-01 (dopamina), APR-01 (neurônio que aprende um padrão), HOM-01, INB-01, CFG-01 (arquivo de parâmetros) e ENG-02 (estado no engrama). O MLP e o CLI não mudam.

## Impacto previsto
- Novos: `src/neuron/neuron.{hpp,cpp}`, `src/synapse/synapse.{hpp,cpp}`.
- Alterados: `CMakeLists.txt` (fontes no `child_core`) e `tests/tests.cpp` (casos novos).
- Dependências: `neuron/` e `synapse/` são o núcleo; não incluem `model/`, `capture/`, `engram/` nem um ao outro. Só a biblioteca padrão.
- O `tests/brain_layout.py` passa a verificar as duas pastas novas. O hash do `reference weights` não muda, porque o MLP fica intocado.
- **Interface pensando no DOP-01:** a sinapse separa o cálculo da mudança proposta pelo STDP da aplicação ao peso. Assim, a próxima entrega intercala a marca de elegibilidade sem reescrever a regra. Nesta entrega a mudança é aplicada direto.
- PAR-01 fica **parcial**: os padrões do neurônio e da sinapse ficam dentro das faixas de [parametros.md](../../../docs/parametros.md), e as demais estruturas vêm depois.

## Autonomia e decisões
- **Autonomia do `cerebro@2`:**
  - Discretização, passo de tempo e parâmetros iniciais dentro de faixas publicadas, sem mudar o comportamento observável.
  - Organização interna.
- **Decisões já tomadas no contrato:** float32, determinismo (sem `<random>` para pesos) e regras locais.
- **Escolhas técnicas desta entrega** (dentro da autonomia; registrar na diferença plano × resultado):
  - Euler explícito com passo fixo; sugerido 0,1 ms.
  - STDP em pares com traços exponenciais.
  - Padrões iniciais:
    - Neurônio: Brette & Gerstner 2005, isto é, C 281 pF, g_L 30 nS, E_L −70,6 mV, V_T −50,4 mV, Δ_T 2 mV, a 4 nS, τ_w 144 ms, b 80,5 pA, V_reset = E_L, V_peak 20 mV.
    - Sinapse: Song et al. 2000, isto é, τ+ = τ− = 20 ms, A+ 0,005·w_max, A−/A+ 1,05, corte rígido em [0, w_max].
- Nenhuma decisão humana aberta.

## Contexto técnico
- **Fontes:**
  - [cerebro v002](../../contracts/cerebro/v002.json) (NEU-01, SIN-01, PAR-01).
  - [parametros.md §1 e §2](../../../docs/parametros.md): equações, faixas e os conjuntos de Naud, úteis para conferir padrões.
  - [architecture.md](../../system/architecture.md) (estrutura-alvo e regra de dependências).
  - [tests/tests.cpp](../../../tests/tests.cpp) (harness `CHECK`/`run`).
  - [CMakeLists.txt](../../../CMakeLists.txt).
- **Proteção existente:** o harness e o `brain_layout`; nenhum teste cobre neurônio ou sinapse.
- **Regras do projeto:** `-Wall -Wextra -Wpedantic` sem avisos, determinismo, identificadores em inglês e documentação em português.
- **Cada executor:**
  - carrega `clean-code` e lê `references/architecture.md` (neurônio e sinapse no círculo interno, sem E/S);
  - confirma as fontes atuais;
  - usa Graphify só se necessário.

## Tasks

### Task 01 — Neurônio AdEx em `src/neuron/`
**Regras:** NEU-01 (e PAR-01 parcial)
**Modo:** TDD
**Responsável:** a atribuir
**Arquivos:** `src/neuron/neuron.hpp`, `src/neuron/neuron.cpp`, `CMakeLists.txt` (uma linha em `child_core`), `tests/tests.cpp` (um caso novo, `adex neuron`)
**Depende de:** —
**Verificação:**
- Risco vital: a dinâmica do neurônio é a base de todas as regras seguintes. Um erro de sinal, de unidade ou de reinício invalida o APR-01 em silêncio.
- Lacuna: nenhuma proteção.
- Um caso `adex neuron` cobre os quatro aceites do NEU-01:
  - repouso sem corrente;
  - disparos repetidos, com os primeiros intervalos crescendo;
  - reinício de V e aumento de w a cada disparo;
  - mesma entrada, mesma sequência.
- Ciclo vermelho → verde.
**Instrução para o executor:**
> Spec master/0008 — Task 01: implementar o neurônio de ponto AdEx (contrato `cerebro@2`, NEU-01) em `src/neuron/`, com arquivo principal `neuron`.
> - **Interface mínima:** parâmetros em struct com unidades nos nomes ou comentários, estado (V, w) legível, e um passo `step(corrente, dt)` que diz se disparou.
> - **Padrões:** os de Brette & Gerstner 2005 em `docs/parametros.md` §1. Euler com passo fixo.
> - **Teste antes:** em `tests/tests.cpp`, escrever o caso `adex neuron` cobrindo os quatro aceites do NEU-01, vê-lo falhar e então implementar.
> - **Limites:** sem dependência de `model/`, `capture/`, `engram/` ou `synapse/`; float32; sem `<random>`; compilar sem avisos; não tocar no MLP nem no CLI.
> - **Ao terminar:** rodar `ctest` e registrar evidência do NEU-01 com `record-evidence.py`.
> - **Relatório:** fontes com linhas, evidência e valores escolhidos (passo, padrões). Não escrever estado, specs nem índices.

### Task 02 — Sinapse com STDP em `src/synapse/`
**Regras:** SIN-01 (e PAR-01 parcial)
**Modo:** TDD
**Responsável:** a atribuir
**Arquivos:** `src/synapse/synapse.hpp`, `src/synapse/synapse.cpp`, `CMakeLists.txt` (uma linha em `child_core`), `tests/tests.cpp` (um caso novo, `stdp synapse`)
**Depende de:** Task 01. As duas tarefas editam o `CMakeLists.txt` e o `tests/tests.cpp`, por isso rodam em série. A sinapse não depende do código do neurônio.
**Verificação:**
- Risco vital: o sinal e a janela do STDP decidem o que a rede aprende; um sinal trocado faz aprender o oposto.
- Lacuna: nenhuma proteção.
- Um caso `stdp synapse` cobre os quatro aceites do SIN-01:
  - pré antes de pós aumenta o peso, e o inverso reduz;
  - o efeito diminui com o intervalo (por exemplo, 5, 20 e 60 ms);
  - o peso fica em [0, w_max] mesmo com muitos pares no mesmo sentido;
  - sem disparos, o peso não muda.
- Ciclo vermelho → verde.
**Instrução para o executor:**
> Spec master/0008 — Task 02: implementar a sinapse com STDP em pares (contrato `cerebro@2`, SIN-01) em `src/synapse/`, com arquivo principal `synapse`.
> - **Entradas:** a sinapse recebe só os eventos de disparo pré e pós com o tempo, mais o decaimento dos traços. Não conhece o neurônio.
> - **Cálculo separado da aplicação:** separar a mudança proposta pelo STDP da aplicação ao peso, para que o DOP-01 possa intercalar a elegibilidade depois. Nesta entrega, aplicar direto.
> - **Padrões:** os de Song et al. 2000 em `docs/parametros.md` §2. Corte rígido em [0, w_max].
> - **Teste antes:** escrever o caso `stdp synapse` cobrindo os quatro aceites do SIN-01, vê-lo falhar e então implementar.
> - **Limites:** os mesmos da Task 01.
> - **Ao terminar:** rodar `ctest` e registrar evidência do SIN-01.
> - **Relatório:** fontes com linhas e evidência. Não escrever estado, specs nem índices.

## Validação e integração
- **Critérios da entrega:**
  - NEU-01 e SIN-01 implementados, com evidência corrente;
  - `ctest` verde, inclusive `brain_layout` com as pastas novas e `reference weights` com o hash inalterado;
  - build do zero sem avisos.
- **Gates:** `validate-lifecycle.py . --ready cerebro/NEU-01` e `--ready cerebro/SIN-01`.
- **Consolidação pelo coordenador:**
  - `.centaur/state/cerebro.json` (NEU-01, SIN-01, PAR-01 parcial);
  - registro em `.centaur/implements/`;
  - linha "Onde estamos" do README, que passa a ter neurônio no código.
- **Integração:** branch e PR, como na entrega anterior, quando autorizado. Esta spec depende do `cerebro@2`, que está no PR #2; integrar depois dele ou na mesma branch.

## Diferença entre plano e resultado
- Implementação: [0007](../../implements/0007/README.md). Seguiu o plano: Euler com 0,1 ms, padrões de BG05 e Song 2000.
- Sinapse com traços decaídos sob demanda (só a cada evento).
- Documentos atualizados (README, `AGENTS.md`, `architecture.md`). Por isso, e pelas mudanças em `tests/tests.cpp` e `CMakeLists.txt`, as evidências de todas as regras afetadas foram refeitas.
- Entregue no PR #2, a pedido do usuário. Falta o merge.

## Checklist de conclusão
- [x] Task 01 — Neurônio AdEx em `src/neuron/`
- [x] Task 02 — Sinapse com STDP em `src/synapse/`

## Execução
/centaur-driven-run master/0008

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
