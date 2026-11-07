# [0011] O cérebro integrado: córtex, cerebelo, dopamina como surpresa, dendritos, hipocampo, neurogênese e CLI

**Data:** 2026-11-07
**Status:** Concluído
**Modo:** TDD (casos em `tests/brain_tests.cpp` escritos com o código e calibrados por medições piloto)
**Spec:** `.centaur/specs/0012/` (mestre) e as filhas 0013 a 0017

## Solicitação
"Pode trabalhar no projeto até que a base esteja totalmente concluída e funcional, precisa ser capaz de expandir e criar neurônios virtuais se necessário (Deve ter uma váriavel de ambiente para esta e outras versões necessárias)" (/goal).

## O que foi feito
- **Encéfalo (`src/brain/`):** `Brain::perceive` apresenta cada byte por 10 ms simulados e faz, em ordem:
  1. o cerebelo aprende o erro da previsão anterior;
  2. o hipocampo guarda o episódio;
  3. a dopamina recebe a surpresa inesperada;
  4. o tálamo dispara e o córtex integra;
  5. o cerebelo prevê o próximo byte;
  6. a neurogênese avalia se é preciso crescer.

  A pasta também tem os parâmetros com faixa (`parameters`), a política de neurogênese e a serialização por `persist()` (`archive.hpp`).
- **Córtex (`src/cortex/`):** células piramidais AdEx com sinapses tálamo-corticais STDP de três fatores, escala homeostática, segmentos distais com platô NMDA e interneurônios com fan-in e fan-out fixos. `grow()` cria células jovens.
- **Cerebelo (`src/cerebellum/`):** grânulos com 4 entradas e k-WTA de 5%, unidades de Purkinje e regra delta local; `grow()` acrescenta grânulos.
- **Hipocampo (`src/hippocampus/`):** reservatório de episódios, `sleep` embaralhado e completação por sobreposição.
- **Dendritos (`src/neuron/dendrite.*`):** pico de cálcio não monotônico (XOR) e segmento distal com permanências e plasticidade estrutural.
- **Tálamo:** código esparso por byte; `set_rate` sorteia de novo o intervalo pendente.
- **Engrama:** seção 2 `brain` (opaca).
- **CLI:**
  - o cérebro é o padrão, e o MLP continua com `--engine mlp`;
  - comandos novos: `sleep`, `grow`, `params` e `bench`;
  - variáveis `CHILD_NEUROGENESIS`, `CHILD_MAX_NEURONS`, `CHILD_PARAMS` e `CHILD_ENGINE`.
- **Testes:**
  - `tests/harness.hpp`, com filtro por nome (`child_tests "sleep protects"`);
  - 10 casos novos em `tests/brain_tests.cpp`;
  - `cli_brain.sh`, `cli_branching.sh` nos dois motores, `parameters_doc.py` e `readme_example.sh`.
- **Documentos:**
  - README, com comandos e variáveis;
  - AGENTS, CONTRIBUTING, arquitetura (reescrita), síntese (situação) e parametros §7 (registro);
  - pesquisa nova em `docs/pesquisa-degraus.md`.

## Decisões técnicas
- **Cerebelo lê tálamo e córtex:** com só o córtex, o acerto no `abcabd` era de 25%.
- **Tálamo a 300 Hz:** a 150 Hz, o código cortical ficava ruidoso demais para separar contextos.
- **Reforço a cada 50 ms com a dopamina integrada** e propostas de STDP abaixo de 10⁻⁷ ignoradas: a vazão subiu de ~400 para ~600 B/s.
- **Fan-in e fan-out fixos nos interneurônios:** com probabilidades fixas, o custo crescia com o quadrado do tamanho.
- **Achado registrado como limite:** os segmentos dendríticos não mudam o resultado do `abcabd`. A separação vem da história do córtex.
