# [0002] Engrama em seções versionadas

**Data:** 2026-11-02
**Status:** Concluído
**Modo:** TDD
**Spec:** `.centaur/specs/0005/` — Task 01

## Solicitação
"pode executar todas" (spec criada a partir de "Crie specs para corrigir cada ponto", ponto 4).

## Contexto
O checkpoint formato 1 era um bloco fixo do MLP. Cada peça nova do cérebro exigiria outro formato e quebraria os clones.

## O que foi feito
- `src/store/` passou a ser `src/engram/` (arquivo principal `engram`); API pública inalterada (`Snapshot`, `create`, `fork`, `save`, `load`).
- Formato 2: cabeçalho (assinatura, versão, linhagem), contagem de seções, seções com tipo/versão/tamanho e checksum FNV-1a. O MLP é a seção tipo 1.
- Tipo ou versão de seção desconhecidos são recusados nomeando o tipo; o formato 1 é recusado com instrução de recriar a base.

## Arquivos modificados
- `src/store/store.{hpp,cpp}` → `src/engram/engram.{hpp,cpp}` — formato 2
- `src/main.cpp`, `CMakeLists.txt` — include e fonte
- `tests/tests.cpp` — caso do ARM-01 ampliado (seção 99, formato 1); correção da base do FNV-1a nos testes; nova constante do `reference weights`
- `AGENTS.md` — camada Engrama

## Decisões técnicas
- Ciclo TDD: os dois CHECKs novos falharam no formato 1 (vermelho) e passaram no formato 2.
- O teste reescreve o arquivo e refaz o checksum, para garantir que a recusa vem da seção e não da corrupção.
- Durante a depuração, a constante inicial do FNV-1a nos testes estava errada (faltava um dígito). Isso não afetava a detecção, mas o nome "FNV-1a" no `reference weights` (master/0003) estava incorreto. Corrigido, e o hash de referência foi refeito (0x6213932d187d6039).

## Como validar
`ctest --test-dir build`; `child info` num `.child` antigo deve recusar com "recreate the base".

## Contrato e realização
`engrama@1` ENG-01: `src/engram/engram.cpp:17-24, 89-187`, `tests/tests.cpp:128-209`, ev-18157f5518a24df5931aa15f894dcdec. Implementada, verificada, entrega local. Evidências do `base@2` refeitas: CAP-01, PRO-01, PRO-02, ARM-01, RAM-01 (2), RNF-01; REP-01 e REP-02 também. ENG-02 segue ausente (depende do neurônio).

## Resultado da validação
`ctest` 2/2; `child_tests` 7 casos ok; `.child` real do formato 1 recusado; init/fork/feed/info/sample funcionam.
