# 2026 Engrama em seções versionadas

**Data:** 2026-11-02
**Status:** Concluída
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** engrama@1
**Regras:** ENG-01
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Crie specs para corrigir cada ponto" (ponto 4: formato do engrama em seções versionadas e com estado dinâmico, desenhado antes do primeiro neurônio)

## Objetivo e demonstração
Antes: o checkpoint v1 é um bloco fixo (configuração do MLP + vetor de floats). Toda peça nova do cérebro exigiria um formato novo e quebraria os clones. Depois: o engrama tem cabeçalho com linhagem e uma sequência de seções (tipo, versão, tamanho, dados). O MLP vira uma seção, e o estado dinâmico do ENG-02 entra depois como outra seção, sem mudar o formato.

Demonstração: `child init/fork/feed/info/sample` funcionam como hoje; um arquivo com seção desconhecida falha com mensagem que nomeia o tipo.

**Fora desta entrega:** ENG-02 (estado dinâmico). Ele depende de `cerebro/NEU-01` e `SIN-01`, ainda sem código, e vira spec própria quando o neurônio existir.

## Impacto previsto
- `src/store/` passa a ser `src/engram/`, seguindo a estrutura-alvo.
- `kVersion` sobe; o CLI só troca o include.
- Este é o "mudar o formato" que a autonomia do `base@2` não permitia: o contrato `engrama@1` é a decisão. Os aceites do ARM-01 e do RAM-01 continuam valendo e as evidências do `base@2` serão refeitas, porque os arquivos mudam.

## Autonomia e decisões
Autonomia: códigos de tipo, tamanhos dos campos e checksum. Decisão tomada: os `.child` v1 são recusados com mensagem que indica recriar a base, sem conversão. Contrato aprovado. Aprovado por Dalistor em 2026-11-02 ("Aprovo todas as suas recomendações").

## Contexto técnico
Fontes: [store.hpp](../../../src/store/store.hpp), [store.cpp](../../../src/store/store.cpp), [main.cpp](../../../src/main.cpp), testes de ARM-01 e RAM-01 em [tests.cpp](../../../tests/tests.cpp), [tests/cli_branching.sh](../../../tests/cli_branching.sh), [estado do base](../../state/base.json). O executor carrega `clean-code` (`references/architecture.md`: o engrama depende só de `Config` do modelo, nunca o contrário) e confirma as fontes; Graphify não é necessário.

## Tasks

### Task 01 — Formato em seções em `src/engram/`
**Regras:** ENG-01 (preservando base/ARM-01 e base/RAM-01)
**Modo:** TDD
**Responsável:** a atribuir
**Arquivos:** `src/store/*` → `src/engram/engram.hpp`, `src/engram/engram.cpp`; `src/main.cpp` (include); `CMakeLists.txt`; `tests/tests.cpp` (casos de ARM-01 e RAM-01)
**Depende de:** —
**Verificação:** risco vital: integridade e persistência do engrama. Proteção existente a reaproveitar: os casos de ARM-01 e RAM-01 e o `cli_branching.sh`. Lacuna concreta: seção desconhecida aceita em silêncio. Ampliar o caso do ARM-01 com esse cenário (vermelho antes, verde depois); não duplicar casos.
**Instrução para o executor:**
> Spec master/0005 — Task 01: mover o armazenamento para `src/engram/` (arquivo principal `engram`), com cabeçalho (assinatura, versão do formato, linhagem), seções (tipo u32, versão u32, tamanho u64, dados) e checksum, mantendo escrita `.tmp` + `rename`. Gravar o MLP como uma seção. Tratar os `.child` v1 conforme a decisão aprovada no contrato. Ampliar o teste do ARM-01 com seção desconhecida → erro que nomeia o tipo. Rodar `ctest`. Registrar evidência do ENG-01 e reportar quais evidências do `base@2` ficaram desatualizadas. Não escrever estado nem índices.

## Validação e integração
`--ready engrama/ENG-01` sem pendências, `ctest` verde, evidências do `base@2` refeitas pelo coordenador. Commit em `main` quando autorizado.

## Diferença entre plano e resultado
- Implementação: [0002](../../implements/0002/README.md). Seguiu o plano.
- Impacto extra: correção da base do FNV-1a nos testes, que refez o hash do REP-02 (master/0003).
- Evidências do `base@2` refeitas e estado `base.json` apontando para `src/engram/`.
- `AGENTS.md` atualizado (camada Engrama).
- Entrega local; falta o commit.
- Integrada no `main` pelo merge do PR #2 (508d00f); `--complete` sem pendências.

## Checklist de conclusão
- [x] Task 01 — Formato em seções em `src/engram/`

## Execução
/centaur-driven-run master/0005

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec.
