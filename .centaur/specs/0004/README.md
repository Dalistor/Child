# 2026 Documentação alinhada à síntese da pesquisa

**Data:** 2026-11-02
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** —
**Regras:** —
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Crie specs para corrigir cada ponto" (ponto 3: `architecture.md`, `AGENTS.md` e README divergem do roteiro e da estrutura da `sintese.md`)

## Objetivo e demonstração
Antes:
- A `architecture.md` (Parte 2) põe os dendritos NMDA/cálcio como próximo passo e não tem `interneuron/` nem `cerebellum/`.
- O README repete o roteiro antigo.

Depois:
- A [sintese.md](../../../docs/sintese.md) é a fonte do roteiro.
- A `architecture.md` traz a estrutura-alvo completa, com o degrau de cada pasta, e aponta para a síntese em vez de repetir o roteiro.
- `AGENTS.md` e README trazem os mesmos nomes de pastas.

Demonstração: `grep -n "interneuron\|cerebellum" AGENTS.md README.md .centaur/system/architecture.md` encontra as três; não há dois roteiros diferentes.

## Impacto previsto
Só documentação: `.centaur/system/architecture.md`, `AGENTS.md`, `README.md`. A [master/0006](../0006/README.md) e a Task 01 da [master/0003](../0003/README.md) também editam o README; executar em série.

## Autonomia e decisões
O rascunho `cerebro` v002 aparece como "rascunho", nunca como aprovado. Contratos e pastas não implementadas são marcados como planejados.

## Contexto técnico
Fontes: [sintese.md](../../../docs/sintese.md) (roteiro de degraus 0 a 5 e tabela base × extensão), [architecture.md](../../system/architecture.md), [AGENTS.md](../../../AGENTS.md), [README.md](../../../README.md). Sem código; Graphify não é necessário.

## Tasks

### Task 01 — Alinhar arquitetura, AGENTS e README
**Regras:** —
**Modo:** direto
**Responsável:** a atribuir
**Arquivos:** `.centaur/system/architecture.md`, `AGENTS.md`, `README.md`
**Depende de:** —
**Verificação:** inspeção e o `grep` da demonstração; links relativos abrem
**Instrução para o executor:**
> Spec master/0004 — Task 01: na Parte 2 da `architecture.md`, trocar o roteiro próprio por um resumo de uma linha por degrau com link para a `sintese.md` e acrescentar `src/interneuron/` (degrau 0) e `src/cerebellum/` (degrau 2) à estrutura-alvo, com a regra de dependências. No `AGENTS.md` (Visão Geral e Arquitetura de Camadas) e na tabela "Onde estamos" do README, usar os mesmos degraus e pastas, citando os contratos em rascunho (`cerebro` v002, `reprodutibilidade`, `engrama`, `base-aberta`) como rascunho. Não alterar contratos nem `docs/`.

## Validação e integração
Revisão humana. Commit em `main` quando autorizado.

## Diferença entre plano e resultado
- Implementação: [0004](../../implements/0004/README.md).
- Os contratos novos já estavam aprovados ao executar; só o `cerebro` v002 aparece como rascunho.
- Parte 1 da arquitetura também atualizada (engrama formato 2, build portátil).
- Aguarda revisão humana e commit.

## Checklist de conclusão
- [x] Task 01 — Alinhar arquitetura, AGENTS e README

## Execução
/centaur-driven-run master/0004

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec.
