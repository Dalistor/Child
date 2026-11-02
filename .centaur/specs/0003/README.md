# 2026 Build reprodutível entre máquinas

**Data:** 2026-11-02
**Status:** Concluída
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** reprodutibilidade@1
**Regras:** REP-01, REP-02
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Crie specs para corrigir cada ponto" (ponto 2: `-march=native` sem `-ffp-contract=off` pode dar pesos diferentes entre máquinas com a mesma semente)

## Objetivo e demonstração
Antes: o `CMakeLists.txt` compila com `-O3 -march=native`. Isso permite fundir multiplicação e soma (FMA) conforme a CPU, e nenhum teste compara os pesos com um valor fixo. Depois: o build padrão é portátil, com `-ffp-contract=off`; o `-march=native` vira opção. Um teste compara o hash dos pesos produzidos por uma semente e um texto conhecidos com um valor gravado.

Demonstração: `cmake --build build -v` mostra as flags; `ctest --test-dir build` passa, inclusive o teste de referência.

## Impacto previsto
`CMakeLists.txt` e `tests/tests.cpp`. Pode reduzir a velocidade de aprendizado medida (~9 mil bytes/s), que é um limite observado e não requisito. Medir e registrar a diferença. Não muda o formato do checkpoint nem o comportamento das regras do `base@2`.

## Autonomia e decisões
Autonomia do contrato: nome da opção e texto hasheado. Contrato aprovado. Aprovado por Dalistor em 2026-11-02 ("Aprovo todas as suas recomendações").

## Contexto técnico
Fontes: [CMakeLists.txt](../../../CMakeLists.txt), [tests/tests.cpp](../../../tests/tests.cpp) (o PRO-02 já testa "mesma semente, mesmos parâmetros" dentro de um mesmo build), [rng.hpp](../../../src/model/rng.hpp). O executor carrega `clean-code` e confirma as fontes atuais; Graphify não é necessário.

## Tasks

### Task 01 — Build padrão portátil e opção nativa
**Regras:** REP-01
**Modo:** direto
**Responsável:** a atribuir
**Arquivos:** `CMakeLists.txt`, `README.md` e `AGENTS.md` (só a linha do comando de compilação)
**Depende de:** —
**Verificação:** `cmake --build build -v` (inspeção das flags), build sem avisos, `ctest --test-dir build`; medir bytes/s antes e depois
**Instrução para o executor:**
> Spec master/0003 — Task 01: no build Release padrão, trocar `-march=native` por flags portáveis e acrescentar `-ffp-contract=off`. Criar uma opção do CMake, desligada por padrão, que reativa `-march=native`, e documentá-la como não reprodutível entre máquinas. Manter `-Wall -Wextra -Wpedantic` sem avisos. Registrar evidência do REP-01 com `record-evidence.py`. Reportar a diferença de bytes/s. Não escrever estado nem índices.

### Task 02 — Hash de referência dos pesos
**Regras:** REP-02
**Modo:** direto
**Responsável:** a atribuir
**Arquivos:** `tests/tests.cpp`
**Depende de:** Task 01
**Verificação:** risco vital: reprodutibilidade dos pesos, que é a promessa central de uma base clonável. Proteção existente: PRO-02, só dentro do mesmo build. Lacuna: nada compara com um valor fixo. Ampliar o caso do PRO-02 com o hash de referência, em vez de criar outro arquivo; conferir que alterar a inicialização faz o caso falhar.
**Instrução para o executor:**
> Spec master/0003 — Task 02: em `tests/tests.cpp`, depois do caso de determinismo existente, criar uma base com semente fixa, ensinar um texto fixo por uma época e comparar o FNV-1a dos parâmetros com uma constante gravada; na falha, imprimir o hash obtido. Gerar a constante no build portátil da Task 01. Registrar evidência do REP-02. Não alterar o comportamento do modelo.

## Validação e integração
`validate-lifecycle.py . --ready reprodutibilidade/REP-01` e `--ready reprodutibilidade/REP-02` sem pendências; `ctest` verde. Commit em `main` quando autorizado.

## Diferença entre plano e resultado
- Implementação: [0001](../../implements/0001/README.md).
- O teste de referência virou um caso próprio (`reference weights`) ao lado do PRO-02, e não uma ampliação dele, porque testa outra coisa: o valor fixo, não a igualdade entre duas instâncias.
- Custo real: −7% de bytes/s.
- Confirmado que as flags antigas mudavam os pesos.
- Durante a master/0005, a base do FNV-1a no teste foi corrigida (faltava um dígito) e a constante virou 0x6213932d187d6039; evidências refeitas.
- Entrega local; falta o commit (integração não autorizada).
- Integrada no `main` pelo merge do PR #2 (508d00f); `--complete` sem pendências.

## Checklist de conclusão
- [x] Task 01 — Build padrão portátil e opção nativa
- [x] Task 02 — Hash de referência dos pesos

## Execução
/centaur-driven-run master/0003

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec.
