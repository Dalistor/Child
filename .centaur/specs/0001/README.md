# 2026 Fontes primárias confirmadas e faixas de parâmetros registradas

**Data:** 2026-11-02
**Status:** Concluída
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** —
**Regras:** —
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Crie specs para corrigir cada ponto" (pontos 1 e 6 da análise da pesquisa: faixas publicadas para a autonomia do `cerebro` e confirmação das fontes primárias antes do v002)

## Objetivo e demonstração
Antes: a pesquisa se apoia em resumos de busca, e a cláusula de autonomia do `cerebro` cita "faixas publicadas" que nenhum documento lista. Depois: `docs/parametros.md` traz, para cada parâmetro do AdEx, do STDP, da elegibilidade/dopamina, da homeostase e da inibição, a faixa, a unidade e a fonte lida. As afirmações que sustentam regras do `cerebro` v002 estão marcadas como confirmadas na fonte primária ou corrigidas.

Demonstração: abrir `docs/parametros.md` e conferir que cada linha tem fonte primária com link; a `sintese.md` mostra o grau de confiança atualizado.

## Impacto previsto
Só documentação em `docs/`. Alimenta a decisão de limiares do `cerebro` v002 ([master/0002](../0002/README.md)) e a regra PAR-01. Sem código.

## Autonomia e decisões
Escolher as fontes. Prefira o artigo original ou uma revisão revisada por pares a preprints. Divergência entre fontes é registrada com as duas faixas, não resolvida por preferência.

## Contexto técnico
Fontes atuais: [sintese.md](../../../docs/sintese.md), [pesquisa-cerebro.md](../../../docs/pesquisa-cerebro.md), [ideias-cerebro.md](../../../docs/ideias-cerebro.md), [cerebro v002](../../contracts/cerebro/v002.json). Regra do projeto: pesquisa nova vira arquivo em `docs/` com data e entra no índice. Sem clean-code (não há código); Graphify não é necessário.

## Tasks

### Task 01 — Ler as fontes primárias das regras do `cerebro`
**Regras:** — (sustenta NEU-01, SIN-01, DOP-01, HOM-01, INB-01)
**Modo:** direto
**Responsável:** a atribuir
**Arquivos:** `docs/parametros.md` (novo)
**Depende de:** —
**Verificação:** inspeção: cada parâmetro com faixa, unidade, fonte e trecho que a justifica
**Instrução para o executor:**
> Spec master/0001 — Task 01: ler, no texto completo, pelo menos Brette & Gerstner 2005 (AdEx), Bi & Poo 1998 e Song, Miller & Abbott 2000 (STDP), Frémaux & Gerstner 2016 (três fatores e elegibilidade), Turrigiano 2008 ou Abbott & Nelson 2000 (escala sináptica) e Vogels et al. 2011 (plasticidade inibitória). Criar `docs/parametros.md` com uma tabela por estrutura (neurônio, sinapse, dopamina, homeostase, inibição): parâmetro, unidade, faixa, valor típico e fonte com link. Registrar divergências. Não alterar contratos, código nem estado; reportar fontes não acessíveis.

### Task 02 — Atualizar a confiança da síntese e o índice
**Regras:** —
**Modo:** direto
**Responsável:** a atribuir
**Arquivos:** `docs/sintese.md` (seção "Grau de confiança"), `docs/README.md`
**Depende de:** Task 01
**Verificação:** inspeção: toda afirmação conferida na Task 01 está marcada como confirmada ou corrigida, e o índice lista `parametros.md`
**Instrução para o executor:**
> Spec master/0001 — Task 02: com base na Task 01, mover para "Firme" o que foi confirmado na fonte primária e corrigir o que divergir. Acrescentar `parametros.md` com data ao índice `docs/README.md`. Propor, sem aplicar, valores para os limiares em aberto do `cerebro` v002 (N, R, T, taxa-alvo e tolerância), com justificativa, numa seção "Proposta de limiares" de `docs/parametros.md`.

## Validação e integração
Revisão humana do documento. Integração por commit em `main` quando autorizado. Não há gate de contrato.

## Diferença entre plano e resultado
- Implementação: [0006](../../implements/0006/README.md).
- Task 01 delegada a um subagente; acesso parcial a Vogels 2011 (suplemento) e Izhikevich 2007 (pago), sem acesso a van Rossum 2000 e Brunel 2000, tudo marcado no documento.
- Duas correções da síntese: a estabilidade do STDP (vale para redes, não para um neurônio isolado com teto e depressão dominante) e a escala de tempo da homeostase em modelos.
- Aguarda revisão humana e commit; a proposta de limiares aguarda decisão na master/0002.

- Integrada no `main` pelo merge do PR #2 (508d00f). O `--complete` não se aplica: a spec não declara regras de comportamento próprias (ou depende de uma que não declara). Concluída por aceite humano em 2026-11-02 ("Aceito, pode marcar as quatro como concluídas").

## Checklist de conclusão
- [x] Task 01 — Ler as fontes primárias das regras do `cerebro`
- [x] Task 02 — Atualizar a confiança da síntese e o índice

## Execução
/centaur-driven-run master/0001

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec.
