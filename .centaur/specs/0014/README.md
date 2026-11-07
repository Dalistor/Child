# 2026 Hipocampo e sono: aprender B sem estragar A

**Data:** 2026-11-07
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** hipocampo@1
**Regras:** HIP-01, HIP-02
**Spec mestre:** master/0012
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Pode trabalhar no projeto até que a base esteja totalmente concluída e funcional, precisa ser capaz de expandir e criar neurônios virtuais se necessário (Deve ter uma váriavel de ambiente para esta e outras versões necessárias)" (/goal de 2026-11-07)

## Objetivo e demonstração
- **HIP-01:** A aprendido fica em 0,57 nat. Depois de 3 épocas de B:
  - sem sono, A sobe a 1,64;
  - com 2 ciclos de sono por época, A fica em 0,36, e B, em 0,39.
- **HIP-02:** 200 de 200 episódios recuperados a partir de metade do padrão.

## Decisões
- Amostragem de reservatório, com capacidade de 2048 episódios.
- O sono repete os episódios embaralhados para o cerebelo. O córtex não é repetido.


## Checklist de conclusão
- [x] Task 01 — Episódios, sono e completação (HIP-01, HIP-02)

## Execução
/centaur-driven-run master/0014

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
