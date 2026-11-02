# 2026 Contrato `cerebro` v002 fechado e aprovado

**Data:** 2026-11-02
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** cerebro@2
**Regras:** —
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** master/0001
**Solicitação original:** "Crie specs para corrigir cada ponto" (ponto 1: homeostase, inibição, faixas publicadas e critérios numéricos no `cerebro` antes de escrever código)

## Objetivo e demonstração
Antes: o `cerebro` v001 aprovado não tem estabilizador, e o APR-01 não tem números. Depois: o rascunho [v002](../../contracts/cerebro/v002.json) fica sem decisões abertas e aprovado. Com isso, as regras HOM-01, INB-01, PAR-01 e CFG-01 e os limiares do APR-01 passam a valer.

Demonstração: `validate-lifecycle.py` aceita o v002 como `approved` e `--ready cerebro/NEU-01` aponta só falta de implementação, não de contrato.

## Impacto previsto
Só o contrato. Evidências do v001 não existem, então nada é invalidado. O estado `.centaur/state/cerebro.json` nasce na primeira entrega de código, ligado ao v002.

## Autonomia e decisões
Decisões do v002:
1. **Limiares** do APR-01 e do HOM-01: **decididos** (N = 500, R = 5, T = 5000, 5 Hz ±20% em 20 s, τ_hom = 5 s).
2. **Representação numérica**: **decidido float32** (já registrado em `boundaries` do v002), com o build reprodutível da [master/0003](../0003/README.md); o ponto fixo fica para quando houver GPU.
3. **Modelo de inibição**: **decidido um neurônio inibitório genérico** (já registrado em `boundaries` do v002).

Decisões 2 e 3: Aprovado por Dalistor em 2026-11-02 ("Aprovo todas as suas recomendações"). Limiares aprovados por Dalistor em 2026-11-02 ("Aprovo os limiares e pode commitar numa branch com PR"), conforme a proposta de [docs/parametros.md](../../../docs/parametros.md#proposta-de-limiares).

## Contexto técnico
Fontes: [cerebro v001](../../contracts/cerebro/v001.json), [v002](../../contracts/cerebro/v002.json), [ideias-cerebro.md §1.1](../../../docs/ideias-cerebro.md), ciclo normativo de `centaur-driven-graphify` (versão aprovada é imutável; aprovação exige `approval.by/at/reference` reais).

## Tasks

### Task 01 — Preencher os limiares e registrar as decisões
**Regras:** APR-01, HOM-01, PAR-01
**Modo:** direto
**Responsável:** coordenador (skill `centaur-driven-spec`)
**Arquivos:** `.centaur/contracts/cerebro/v002.json`
**Depende de:** master/0001
**Verificação:** `validate-lifecycle.py` sem erros; nenhum N, R, T ou X sem número
**Instrução para o executor:**
> Spec master/0002 — Task 01: substituir os símbolos dos aceites do APR-01 e do HOM-01 pelos valores decididos, esvaziar `decisions` e incluir `approval` com a referência real da decisão do usuário. Não inferir aprovação.

## Validação e integração
`validate-lifecycle.py .` sem erros e v002 aprovado. Commit em `main` quando autorizado.

## Diferença entre plano e resultado
- v002 aprovado, sem decisões abertas. Acrescentado em `boundaries` o motivo da homeostase rápida (Zenke 2013).
- Estado `cerebro.json` passou para `cerebro@2` e o REG-01 foi revalidado (nova evidência).
- Correção do plano: as regras APR-01, HOM-01, INB-01, PAR-01 e CFG-01 estavam listadas aqui, mas esta spec só entrega o contrato que as define. A implementação delas fica com as specs de cada degrau (NEU-01 e SIN-01 já estão na master/0008).
- Integrada no `main` pelo merge do PR #2 (508d00f). O `--complete` não se aplica: a spec não declara regras de comportamento próprias (ou depende de uma que não declara). Fica em revisão até o aceite humano da entrega.
- Próximo passo: spec de implementação do degrau 0 (NEU-01 e SIN-01).

## Checklist de conclusão
- [x] Task 01 — Preencher os limiares e registrar as decisões

## Execução
/centaur-driven-spec (fechar contrato), depois specs de implementação do `cerebro`

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec.
