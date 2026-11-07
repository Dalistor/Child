# 2026 Engrama com estado dinâmico (ENG-02)

**Data:** 2026-11-07
**Status:** Concluída
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** engrama@1
**Regras:** ENG-02
**Spec mestre:** master/0012
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Pode trabalhar no projeto até que a base esteja totalmente concluída e funcional, precisa ser capaz de expandir e criar neurônios virtuais se necessário (Deve ter uma váriavel de ambiente para esta e outras versões necessárias)" (/goal de 2026-11-07)

## Objetivo e demonstração
A seção `brain` guarda o estado inteiro: potenciais, adaptação, condutâncias, traços, marcas de elegibilidade, ganhos, segmentos, episódios, `Rng` e tempo. Salvar depois de 150 bytes e continuar dá os mesmos disparos em 120 de 120 bytes, tanto na base retomada quanto num fork novo.


## Integração
PR #6, com merge no `main` (22d196b) e a CI `build-and-test` verde. `--ready` de cada regra e `--complete master/0017` sem pendências.

## Checklist de conclusão
- [x] Task 01 — persist() em cada peça e seção brain (ENG-02)

## Execução
/centaur-driven-run master/0017

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
