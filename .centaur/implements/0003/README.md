# [0003] Script que verifica os nomes das estruturas do cérebro

**Data:** 2026-11-02
**Status:** Concluído
**Modo:** direto
**Spec:** `.centaur/specs/0007/` — Task 01

## Solicitação
"pode executar todas" (ponto 5: script do REG-01).

## O que foi feito
`tests/brain_layout.py` (Python 3, só biblioteca padrão), registrado no `ctest` como `brain_layout`. Sem Python, o teste não é registrado e o build segue.

## Arquivos modificados
- `CMakeLists.txt` — `find_package(Python3 ... QUIET)` e `add_test(brain_layout)`

## Arquivos criados
- `tests/brain_layout.py` — verificação do REG-01

## Decisões técnicas
Legadas: só `capture` e `model`, porque `store` deixou de existir na master/0005 e `engram/` já cumpre a regra.

## Como validar
`ctest --test-dir build -R brain_layout`; criar `src/foo/bar.cpp` faz falhar nomeando `src/foo`.

## Contrato e realização
`cerebro@1` REG-01: `tests/brain_layout.py:1-25`, `CMakeLists.txt:38-43`, ev-e13fdab19a3d474a86b3825114409636. Implementada, verificada, entrega local.

## Resultado da validação
`ctest` 3/3; o caso negativo numa cópia de `src/` falhou como esperado.
