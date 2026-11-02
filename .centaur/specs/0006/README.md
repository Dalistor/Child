# 2026 Repositório pronto para ser clonado

**Data:** 2026-11-02
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** base-aberta@1
**Regras:** ABR-01, ABR-02, ABR-03
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** master/0004
**Solicitação original:** "Crie specs para corrigir cada ponto" (ponto 5: licença, guia de contribuição e exemplo de ponta a ponta)

## Objetivo e demonstração
Antes: não há `LICENSE` nem `CONTRIBUTING.md`, e o README tem comandos soltos. Depois: há licença declarada, um guia de contribuição e um exemplo, do clone ao fork treinado, que roda sem alterações.

Demonstração: executar o exemplo do README num clone limpo.

**Fora desta entrega:** parâmetros sem recompilar (`cerebro/CFG-01`, depende do v002 e do `engrama`) e o script do REG-01 ([master/0007](../0007/README.md)).

## Impacto previsto
`LICENSE` e `CONTRIBUTING.md` (novos) e `README.md`. Executar depois da [master/0004](../0004/README.md), que também edita o README.

## Autonomia e decisões
Decisão tomada: licença **MIT**. Contrato aprovado. Aprovado por Dalistor em 2026-11-02 ("Aprovo todas as suas recomendações"). Executar depois da master/0004.

## Contexto técnico
Fontes: [README.md](../../../README.md), [AGENTS.md](../../../AGENTS.md) (regras que o CONTRIBUTING resume para humanos), [tests/cli_branching.sh](../../../tests/cli_branching.sh) (já exercita init, fork, feed e info; serve de base para o exemplo). Sem lógica de código; Graphify não é necessário.

## Tasks

### Task 01 — Licença, guia e exemplo
**Regras:** ABR-01, ABR-02, ABR-03
**Modo:** direto
**Responsável:** a atribuir
**Arquivos:** `LICENSE`, `CONTRIBUTING.md`, `README.md`
**Depende de:** master/0004
**Verificação:** ABR-01 e ABR-02 por inspeção; ABR-03 executando o exemplo num clone limpo em diretório temporário, com evidência `manual` (comando e saída observada)
**Instrução para o executor:**
> Spec master/0006 — Task 01: criar `LICENSE` com o texto integral da licença aprovada no contrato, e `CONTRIBUTING.md` com compilar e testar, fluxo de contratos e evidências, regras de aprendizado local e determinismo, idioma e como propor um contrato. No README, trocar os comandos soltos por um exemplo numerado, do `git clone` ao `info` mostrando a ramificação treinada e a raiz virgem, e citar a licença e o guia. Executar o exemplo num clone limpo e registrar a evidência do ABR-03. Não alterar código.

## Validação e integração
`--ready` para ABR-01, ABR-02 e ABR-03. Commit em `main` quando autorizado.

## Diferença entre plano e resultado
- Implementação: [0005](../../implements/0005/README.md).
- ABR-03 verificado numa cópia limpa da árvore candidata, não num `git clone` real (as mudanças não estão no remoto); repetir após o push.
- Entrega local; falta o commit.

- Integrada no `main` pelo merge do PR #2 (508d00f). O `--complete` não se aplica: a spec não declara regras de comportamento próprias (ou depende de uma que não declara). Fica em revisão até o aceite humano da entrega.

## Checklist de conclusão
- [x] Task 01 — Licença, guia e exemplo

## Execução
/centaur-driven-run master/0006

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec.
