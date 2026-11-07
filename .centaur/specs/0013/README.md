# 2026 Segmentos dendríticos: XOR num neurônio e contextos de sequência

**Data:** 2026-11-07
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** dendritos@1
**Regras:** DEN-01, DEN-02
**Spec mestre:** master/0012
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Pode trabalhar no projeto até que a base esteja totalmente concluída e funcional, precisa ser capaz de expandir e criar neurônios virtuais se necessário (Deve ter uma váriavel de ambiente para esta e outras versões necessárias)" (/goal de 2026-11-07)

## Objetivo e demonstração
- **DEN-01:** um neurônio com dendrito de cálcio (Gidon 2020) responde a (0,1) e (1,0) e se cala em (0,0) e (1,1). Sem o dendrito, faz E lógico.
- **DEN-02:** treinado em `abcabd`, o cérebro prevê c depois de `dab` e d depois de `cab`, em 0,96 dos casos cada.

## Resultado e limite
- **Ablação:** sem segmentos, 0,96 e 0,95. Os segmentos estão ligados e aprendem (~16 células previstas por byte no treino), mas não são eles que separam os contextos neste critério. Quem separa é a história do córtex: cosseno de 0,02 entre as representações de "b" nos dois contextos.
- **Pendente:** um teste em que os segmentos sejam necessários.


## Checklist de conclusão
- [x] Task 01 — Pico de cálcio e XOR (DEN-01)
- [x] Task 02 — Segmentos distais no córtex e contextos (DEN-02)

## Execução
/centaur-driven-run master/0013

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
