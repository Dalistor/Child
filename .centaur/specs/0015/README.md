# 2026 Neurogênese: neurônios criados quando o cérebro precisa, com teto e variáveis de ambiente

**Data:** 2026-11-07
**Status:** Concluída
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** neurogenese@1
**Regras:** NEO-01, NEO-02
**Spec mestre:** master/0012
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Pode trabalhar no projeto até que a base esteja totalmente concluída e funcional, precisa ser capaz de expandir e criar neurônios virtuais se necessário (Deve ter uma váriavel de ambiente para esta e outras versões necessárias)" (/goal de 2026-11-07)

## Objetivo e demonstração
- **NEO-01:**
  - num texto aleatório, 128 células chegam a 176 (teto), e o cerebelo passa a ler as fibras novas;
  - no `abab`, nada nasce, e desligado também não;
  - as células nascidas disparam;
  - o crescimento é determinístico e o engrama o preserva.
- **NEO-02:** `CHILD_NEUROGENESIS=off`, `CHILD_MAX_NEURONS`, `CHILD_PARAMS` e `CHILD_ENGINE` pelo CLI (`tests/cli_brain.sh`).

## Decisões
- **Quando crescer:** janela de 2048 bytes; nasce um lote quando a surpresa média passa de 1,5 nat e não cai 2% em relação à janela anterior (Draelos 2017; Fritzke 1995).
- **Quanto:** lotes de 32 células, até 1024.
- **Células jovens:** 2 vezes mais plásticas por 20 s (Ge 2007).
- **Escopo das variáveis:** valem para a execução e não reescrevem os parâmetros gravados.


## Integração
PR #6, com merge no `main` (22d196b) e a CI `build-and-test` verde. `--ready` de cada regra e `--complete master/0015` sem pendências.

## Checklist de conclusão
- [x] Task 01 — Política e crescimento do córtex e do cerebelo (NEO-01)
- [x] Task 02 — Variáveis de ambiente no CLI (NEO-02)

## Execução
/centaur-driven-run master/0015

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
