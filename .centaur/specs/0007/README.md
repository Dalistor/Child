# 2026 Script que verifica os nomes das estruturas do cérebro

**Data:** 2026-11-02
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** cerebro@1
**Regras:** REG-01
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Crie specs para corrigir cada ponto" (ponto 5: script do REG-01)

## Objetivo e demonstração
Antes: o REG-01 (aprovado no `cerebro` v001) exige uma verificação por script, que não existe. Depois: um script em Python 3, sem dependências, falha se alguma pasta de `src/` fora da lista de legadas não tiver um arquivo principal de mesmo nome. O script roda no `ctest`.

Demonstração: `ctest --test-dir build` passa. Criar `src/foo/bar.cpp` faz o check falhar nomeando `src/foo`.

## Impacto previsto
Novo `tests/brain_layout.py` (ou nome equivalente) e registro no `CMakeLists.txt`. Legadas explícitas: `capture`, `model`, `store` (e `engram` só depois da master/0005, quando passar a cumprir a regra). Python já é exigido pelo fluxo Centaur. Ele **não** é dependência do build do Child, e o check não deve impedir compilar.

## Autonomia e decisões
Autonomia do `cerebro` v001: organização interna. Esta entrega não abre nenhuma decisão. O contrato está aprovado e pode ser executado já.

## Contexto técnico
Fontes: [cerebro v001](../../contracts/cerebro/v001.json) (REG-01), [CMakeLists.txt](../../../CMakeLists.txt) (padrão do `add_test` do `cli_branching`). O executor carrega `clean-code`; Graphify não é necessário.

## Tasks

### Task 01 — Check do REG-01 no ctest
**Regras:** REG-01
**Modo:** direto
**Responsável:** a atribuir
**Arquivos:** `tests/brain_layout.py` (novo), `CMakeLists.txt`
**Depende de:** —
**Verificação:** o próprio script é a proteção; conferir o caso positivo (repositório atual passa) e o negativo (pasta temporária sem arquivo principal falha) e registrar evidência com `record-evidence.py ... -- ctest --test-dir build -R brain_layout`
**Instrução para o executor:**
> Spec master/0007 — Task 01: escrever um script Python 3 só com a biblioteca padrão que recebe a raiz de `src/`, ignora as pastas legadas listadas nele e falha, nomeando a pasta, quando `src/<x>/` não tem `<x>.hpp` ou `<x>.cpp`. Registrá-lo no `ctest` (se o Python não for encontrado, o teste é pulado e o build não falha). Registrar evidência do REG-01. Não escrever estado nem índices.

## Validação e integração
`validate-lifecycle.py . --ready cerebro/REG-01`. Commit em `main` quando autorizado.

## Diferença entre plano e resultado
- Implementação: [0003](../../implements/0003/README.md).
- Legadas reduzidas a `capture` e `model` (a `store` virou `engram` na master/0005).
- Entrega local; falta o commit.

## Checklist de conclusão
- [x] Task 01 — Check do REG-01 no ctest

## Execução
/centaur-driven-run master/0007

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec.
