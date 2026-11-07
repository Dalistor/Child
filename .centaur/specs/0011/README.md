# 2026 Um neurônio aprende um padrão escondido no ruído (APR-01)

**Data:** 2026-11-07
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** cerebro@2
**Regras:** APR-01
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** master/0008, master/0010
**Solicitação original:** "Pode trabalhar no projeto até que a base esteja totalmente concluída e funcional" (/goal de 2026-11-07)

## Objetivo e demonstração
- **Antes:** neurônio, sinapse, homeostase e inibição passam em seus testes isolados, mas nada mostra um neurônio aprendendo algo útil com elas.
- **Depois:** o caso `pattern learning` mostra um neurônio com 500 sinapses plásticas que passa a disparar só dentro de um padrão de 50 ms, repetido em 1/4 do tempo no meio de ruído de mesma densidade.

Fora desta entrega:
- **CFG-01** (arquivo de parâmetros) e **ENG-02** (estado no engrama): entram com a rede integrada ao CLI.
- **Robustez entre sementes:** a regra pede um neurônio, e a configuração passa em 5 das sementes 1 a 8. O limite fica registrado.

## Impacto previsto
- **Novos:** `src/thalamus/thalamus.{hpp,cpp}`. O gerador de Poisson sai dos testes e vira a célula relé do tálamo, agora por evento: um sorteio por disparo, não por passo.
- **Alterados:**
  - `src/synapse/synapse.{hpp,cpp}`: tempos em `double` (`Millis`). Em float32, 10⁷ ms já têm 1 ms de resolução e degradam a janela do STDP nos treinos longos.
  - `tests/tests.cpp`, `CMakeLists.txt` e `docs/parametros.md` §6.
- **Compatibilidade:** os casos existentes passam e o hash do `reference weights` não muda. Os testes de HOM-01 e INB-01 trocaram de gerador e seguem nas mesmas faixas.

## Autonomia e decisões
- **Autonomia do `cerebro@2`:** parâmetros iniciais dentro de faixas publicadas, discretização e codificação.
- **Escolhas desta entrega:**
  - STDP A+ = 0,02, τ+ = 16,8 ms, τ− = 20 ms, A−/A+ = 1,05;
  - homeostase padrão;
  - 500 aferentes a 20 Hz.
- Nenhuma decisão humana aberta.

## Tasks
### Task 01 — Tálamo como fonte de disparos e tempo em double
**Regras:** — (base do APR-01) · **Modo:** direto · **Arquivos:** `src/thalamus/*`, `src/synapse/*`, `tests/tests.cpp`, `CMakeLists.txt`

### Task 02 — Caso `pattern learning`
**Regras:** APR-01 · **Modo:** TDD · **Arquivos:** `tests/tests.cpp`, `docs/parametros.md`
- **Verificação:**
  - razão ≥ 5 após 500 apresentações;
  - < 5 sem plasticidade;
  - ≥ 5 após 5000;
  - todas as janelas de 20 s depois de 100 s entre 4 e 6 Hz.

## Validação e integração
- `ctest --test-dir build` (child_tests, cli_branching, brain_layout).
- `validate-lifecycle.py --ready cerebro/APR-01` e `--complete master/0011`.
- Integração por PR no `main`.

## Diferença entre plano e resultado
- **Implementação:** [0010](../../implements/0010/README.md).
- **Resultado (semente 5):**
  - 500 apresentações: 19,6 Hz dentro e 0 fora;
  - sem plasticidade: razão 0,5;
  - 5000 apresentações: razão 8,4;
  - janelas entre 4,6 e 5,75 Hz.
- **Desvio:** os padrões exatos de Masquelier (τ− 33,7, A−/A+ 0,85) deixavam o neurônio antisseletivo na maior parte das sementes. Com τ+ = τ− = 20, ele aprendia e depois perdia a seletividade. A combinação escolhida foi a mais robusta das testadas. As variações estão em [parametros.md §6](../../../docs/parametros.md).
- **Gerador com contagem:** o gerador por evento devolve quantos disparos caem no passo. Um processo agregado de 4000 Hz, como o fundo do teste de inibição, dispara mais de uma vez por passo.
- **Evidências:** todas as afetadas foram refeitas, porque mudaram `tests.cpp`, `CMakeLists.txt` e a sinapse.

## Checklist de conclusão
- [x] Task 01 — Tálamo como fonte de disparos e tempo em double
- [x] Task 02 — Caso `pattern learning`

## Execução
/centaur-driven-run master/0011

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
