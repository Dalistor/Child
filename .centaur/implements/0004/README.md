# [0004] Documentação alinhada à síntese da pesquisa

**Data:** 2026-11-02
**Status:** Concluído
**Modo:** direto
**Spec:** `.centaur/specs/0004/` — Task 01

## Solicitação
"pode executar todas" (ponto 3: documentos divergentes da `sintese.md`).

## O que foi feito
- `architecture.md`: Parte 1 atualizada (engrama formato 2, build portátil, nova velocidade). Parte 2 com estrutura-alvo completa (degrau por pasta, `interneuron/` e `cerebellum/`) e roteiro resumido com link para a síntese. Os dendritos NMDA/cálcio passam a ser refinamento depois do degrau 3.
- `AGENTS.md`: visão geral com os degraus, lista de contratos (novos aprovados e `cerebro` v002 em rascunho), estrutura-alvo.
- `README.md`: tabela "Onde estamos" com os degraus 0–5, estrutura-alvo, link da pesquisa para o índice.

## Arquivos modificados
- `.centaur/system/architecture.md`, `AGENTS.md`, `README.md`

## Decisões técnicas
Os contratos que a spec mandava citar "como rascunho" foram aprovados antes da execução; só o `cerebro` v002 aparece como rascunho.

## Como validar
`grep -n "interneuron\|cerebellum" AGENTS.md README.md .centaur/system/architecture.md`; conferir que só a síntese tem o roteiro detalhado.

## Contrato e realização
Sem contrato (documentação). Verificação por inspeção; links relativos conferidos por script.

## Resultado da validação
As três fontes citam as pastas novas; nenhum link relativo quebrado.
