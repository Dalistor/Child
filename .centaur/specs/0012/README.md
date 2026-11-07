# 2026 O cérebro integrado prevê o próximo byte pelo CLI

**Data:** 2026-11-07
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** encefalo@1
**Regras:** TAL-01, CER-01, SUR-01, CLI-01, ESC-01
**Spec mestre:** —
**Specs filhas:** master/0013, master/0014, master/0015, master/0016, master/0017
**Dependências:** master/0011
**Solicitação original:** "Pode trabalhar no projeto até que a base esteja totalmente concluída e funcional, precisa ser capaz de expandir e criar neurônios virtuais se necessário (Deve ter uma váriavel de ambiente para esta e outras versões necessárias)" (/goal de 2026-11-07)

## Objetivo e demonstração
- **Antes:** as peças do cérebro só se ligavam nos testes, e o CLI usava o MLP.
- **Depois:** `child init` cria um cérebro, e `feed`, `sleep`, `grow`, `sample`, `info`, `params` e `bench` funcionam com ele. O fluxo é:
  - tálamo;
  - córtex de AdEx com STDP × dopamina, homeostase, interneurônios e segmentos dendríticos;
  - cerebelo, que prevê o próximo byte;
  - surpresa, que vira dopamina;
  - hipocampo;
  - neurogênese.

O MLP fica com `--engine mlp`, como oráculo.

Esta é a mestre da entrega que conclui a base pedida no /goal. As filhas cobrem um contrato cada:
- dendritos (0013);
- hipocampo (0014);
- neurogênese (0015);
- CFG-01 e PAR-01 (0016);
- ENG-02 (0017).

## Impacto previsto e real
- **Novos:**
  - `src/cortex/`, `src/cerebellum/`, `src/hippocampus/`, `src/brain/` (`brain`, `parameters`, `neurogenesis`, `archive`);
  - `src/neuron/dendrite.*`;
  - o codificador `Thalamus` em `src/thalamus/`;
  - `tests/brain_tests.cpp`, `tests/harness.hpp`, `tests/cli_brain.sh`, `tests/parameters_doc.py` e `tests/readme_example.sh`.
- **Alterados:**
  - `persist()` em `Rng`, `Neuron`, `Synapse`, `Homeostasis`, `Dopamine`, `Interneuron` e `SpikeSource`;
  - `SpikeSource::set_rate` sorteia de novo o intervalo pendente;
  - `Synapse::eligibility` sem exp quando não há marca;
  - engrama com a seção `brain` (tipo 2);
  - `src/main.cpp` reescrito;
  - `cli_branching.sh` nos dois motores;
  - README, CONTRIBUTING, AGENTS, arquitetura, síntese e parametros (§7).
- **Compatibilidade:** os casos do MLP e do núcleo passam, o hash do `reference weights` não muda, e engramas MLP do formato 2 continuam legíveis.

## Decisões dentro da autonomia
- **Mossy fibers:** o cerebelo lê os canais do tálamo e o córtex. Com só o córtex, o `abcabd` ficava em 25%: a 5 Hz, só ~13 células disparam por byte.
- **Tálamo a 300 Hz:** com 150 Hz o código cortical era ruidoso demais para separar contextos (d em 0,80). Com 300 Hz, c e d ficam em 0,96.
- **Reforço por intervalo de 50 ms** com a dopamina integrada no intervalo, em vez de por byte, mais o limiar de mudança desprezível. A vazão subiu de ~400 para ~600 B/s.
- **Conectividade dos interneurônios em contagens fixas:** com probabilidades, 4096 células levavam mais de 11 min por 300 bytes. Com contagens, levam 27 B/s.
- **Dopamina = max(0, surpresa − surpresa esperada):** por isso a dopamina por byte não cai com o treino. O critério do contrato é o novo contra o aprendido.

## Resultado (child_tests)
- **TAL-01:** 291 Hz nos canais do byte e 1,8 Hz nos demais.
- **CER-01:** acerto 0,967 e surpresa 0,54 nat.
- **SUR-01:**
  - surpresa 3,28 → 0,71;
  - dopamina 1,75 no texto novo contra 0,18 no aprendido;
  - correlação de 0,33 com o MLP.
- **CLI-01:** `cli_branching` e `cli_brain` passam.
- **ESC-01:** 538, 121 e 27 B/s com 256, 1024 e 4096 células.

## Validação e integração
- `ctest --test-dir build` (5 testes).
- `tests/readme_example.sh`.
- `validate-lifecycle.py --ready` de cada regra, mais `--complete` das filhas e da mestre.
- PR no `main`.


## Checklist de conclusão
- [x] Task 01 — Tálamo como codificador de bytes (TAL-01)
- [x] Task 02 — Córtex, cerebelo e encéfalo (CER-01)
- [x] Task 03 — Dopamina como surpresa (SUR-01)
- [x] Task 04 — CLI com o cérebro como motor padrão (CLI-01)
- [x] Task 05 — Vazão medida (ESC-01)

## Execução
/centaur-driven-run master/0012

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
