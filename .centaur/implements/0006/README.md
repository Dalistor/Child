# [0006] Fontes primárias confirmadas e faixas de parâmetros registradas

**Data:** 2026-11-02
**Status:** Concluído
**Modo:** direto
**Spec:** `.centaur/specs/0001/` — Tasks 01 e 02

## Solicitação
"pode executar todas" (pontos 1 e 6: faixas publicadas e confirmação das fontes antes do `cerebro` v002).

## O que foi feito
- Task 01 (subagente de pesquisa): `docs/parametros.md` com uma tabela por estrutura, trechos das fontes, confirmação das afirmações da síntese e proposta de limiares.
- Task 02 (coordenador): grau de confiança da `sintese.md` revisto, lição 2 corrigida (estabilidade do STDP) e `parametros.md` no índice.

## Arquivos criados
- `docs/parametros.md`

## Arquivos modificados
- `docs/sintese.md`, `docs/README.md`

## Decisões técnicas
- Valores vistos só em fonte secundária (patente de Izhikevich, código do ModelDB de Vogels) ficam marcados como tal.
- Proposta de limiares, ainda **não decidida**: N = 500 apresentações, R ≥ 5, T = 5000, taxa-alvo de 5 Hz ±20% (média de 20 s), τ_hom = 5 s.

## Contrato e realização
Sem contrato (pesquisa). Alimenta a decisão de limiares do `cerebro` v002 (master/0002).

## Resultado da validação
Revisão do coordenador: as fontes e os trechos estão presentes, e o que não foi confirmado está marcado. Links relativos conferidos.
