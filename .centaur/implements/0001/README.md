# [0001] Build reprodutível entre máquinas

**Data:** 2026-11-02
**Status:** Concluído
**Modo:** direto
**Spec:** `.centaur/specs/0003/` — Tasks 01 e 02

## Solicitação
"pode executar todas" (specs criadas a partir de "Crie specs para corrigir cada ponto").

## Contexto
`-O3 -march=native` sem `-ffp-contract=off` deixava o compilador fundir operações em FMA conforme a CPU. Confirmado: com as flags antigas, a semente 42 gera pesos com hash `0xdbb8b8c8e7da8a9d`; no build portátil, `0x7e20ee7914140083`.

## O que foi feito
- Build padrão portátil (`-O3 -ffp-contract=off`) e opção `CHILD_NATIVE` (OFF) para `-march=native`.
- Teste `reference weights`, que fixa o hash dos pesos de uma semente e um texto conhecidos.

## Arquivos modificados
- `CMakeLists.txt` — flags e opção `CHILD_NATIVE`
- `tests/tests.cpp` — `test_reference_weights`
- `AGENTS.md` — comando de compilação e regra de determinismo
- `README.md` — comentário do comando de compilação

## Decisões técnicas
- `-ffp-contract=off` também se aplica com `CHILD_NATIVE=ON`. Mesmo assim, a opção continua documentada como não reprodutível, porque a vetorização dependente da CPU pode mudar a ordem das somas.
- Custo medido: ~9,2 mil para ~8,6 mil bytes/s (−7%).
- Limitação: a igualdade bit a bit foi verificada só nesta máquina (x86-64, GCC, glibc). `tanh` e `exp` da libm podem variar entre bibliotecas C.

## Como validar
`cmake -S . -B build -G Ninja && cmake --build build -v` (as flags aparecem) e `ctest --test-dir build`.

## Contrato e realização
`reprodutibilidade@1`, REP-01 (`CMakeLists.txt:11-18`, ev-1cf959b3bcac430290e123127e2abc03) e REP-02 (`tests/tests.cpp:82-95`, ev-092bb75dfb6e4dddbb0147d94024e8a9). Implementadas, verificadas e com entrega local (sem commit).

## Resultado da validação
Build do zero sem avisos; `ctest`: 2/2 passaram (`child_tests` com 7 casos, `cli_branching`).
