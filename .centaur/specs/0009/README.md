# 2026 Dopamina como terceiro fator: o STDP só vira mudança de peso se a dopamina chegar a tempo

**Data:** 2026-11-02
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** cerebro@2
**Regras:** DOP-01
**Spec mestre:** —
**Specs filhas:** —
**Dependências:** master/0008
**Solicitação original:** "Crie a spec da dopamina"

## Objetivo e demonstração
Antes: a sinapse (SIN-01) aplica a mudança do STDP direto no peso. Nada decide se aquela coincidência importava. Depois:
- **Marca de elegibilidade:** a sinapse pode registrar a mudança proposta pelo STDP numa marca que decai com o tempo, sem tocar no peso.
- **Dopamina:** `src/dopamine/` produz um sinal de dopamina. Quando ele chega enquanto a marca ainda vale, o peso muda em proporção à marca e à dopamina. Sem dopamina, ou com ela chegando depois de a marca expirar, o peso fica exatamente como estava.

Demonstração: o caso `three-factor plasticity` no `ctest` imprime a mudança de peso para dopamina a 0,2 s, 1 s e 6 s depois do par e sem dopamina. Nos dois primeiros o peso muda, encolhendo com o atraso; nos dois últimos não muda.

Fora desta entrega:
- **Dopamina como surpresa** (degrau 1): de onde vem o sinal ao prever o próximo byte.
- **APR-01, HOM-01 e INB-01.**
- **Dopamina tônica** (basal).
- **Estado no engrama** (ENG-02).

## Impacto previsto
- **Novos:** `src/dopamine/dopamine.{hpp,cpp}`, com o sinal global de dopamina, que decai com τ_d e recebe liberações pontuais.
- **Alterados:**
  - `src/synapse/synapse.{hpp,cpp}`: marca de elegibilidade e reforço pelo nível de dopamina. A interface atual (`on_pre`, `on_post`, `apply`) continua; o modo de três fatores é um caminho a mais.
  - `CMakeLists.txt` e `tests/tests.cpp`, com um caso novo.
- **Dependências:**
  - A sinapse recebe a dopamina como um número; não inclui `dopamine/`. A dopamina não conhece a sinapse.
  - Quem junta os dois é quem simula: por enquanto, o teste. Mais tarde será uma estrutura de rede.
  - Nenhum dos dois conhece `neuron/`, `model/` ou `engram/`.
- **Compatibilidade:** o caso `stdp synapse` (SIN-01) segue passando sem mudança, e o hash do `reference weights` não muda.
- **PAR-01:** ganha τ_e e τ_d, ainda parcial.

## Autonomia e decisões
- **Autonomia do `cerebro@2`:** discretização, passo de tempo e parâmetros dentro de faixas publicadas, sem mudar o comportamento observável. A organização interna também é livre.
- **Escolhas técnicas desta entrega** (dentro da autonomia; registrar na diferença plano × resultado):
  - **τ_e = 1 s:** valor típico de Izhikevich, dentro da faixa de Frémaux & Gerstner, "centenas de ms a ~1 s". Ver [parametros.md §3](../../../docs/parametros.md).
  - **τ_d = 0,2 s:** Izhikevich, visto só na patente, e marcado assim.
  - **Expiração da marca:** a marca decai exponencialmente e **expira em 5·τ_e** (5 s, menos de 1% do valor inicial), quando passa a valer exatamente zero. Sem esse corte, uma exponencial nunca chega a zero, e o segundo aceite do DOP-01 ("dopamina depois da marca expirar não deixa alteração persistente") não teria um ponto observável. A janela cobre o atraso de 1 a 3 s que Izhikevich tolera.
  - **Dopamina basal zero.** Com dopamina tônica, a marca viraria mudança de peso o tempo todo, o que contraria "sem dopamina, não há alteração". A dopamina tônica fica para quando houver um alvo explícito.
  - **Regra de reforço:** `dw/dt = e(t) · d(t)`, como em Frémaux & Gerstner (Eq. 6), integrada com o passo da simulação. O executor pode trocar por uma forma fechada equivalente.
- Nenhuma decisão humana aberta.

## Contexto técnico
- **Fontes:**
  - [cerebro v002](../../contracts/cerebro/v002.json) (DOP-01, PAR-01).
  - [parametros.md §3](../../../docs/parametros.md) (τ_e, τ_d, equação de referência).
  - [synapse.hpp](../../../src/synapse/synapse.hpp) e [synapse.cpp](../../../src/synapse/synapse.cpp): já separam a proposta (`on_pre`/`on_post`) da aplicação (`apply`), e foi para isso.
  - Caso `stdp synapse` em [tests.cpp](../../../tests/tests.cpp).
  - [master/0008](../0008/README.md).
- **Proteção existente:**
  - O caso `stdp synapse` cobre o sinal e a janela do STDP, e é reaproveitado.
  - O `brain_layout` passa a verificar `src/dopamine/`.
- **Regras do projeto:** float32, determinismo, sem `<random>` para pesos, sem avisos com `-Wall -Wextra -Wpedantic`, identificadores em inglês.
- **Cada executor:**
  - carrega `clean-code` e lê `references/architecture.md` (sinapse e dopamina no círculo interno, sem se conhecerem);
  - confirma as fontes atuais;
  - usa Graphify só se necessário.

## Tasks

### Task 01 — Marca de elegibilidade e sinal de dopamina
**Regras:** DOP-01 (e PAR-01 parcial)
**Modo:** TDD
**Responsável:** a atribuir
**Arquivos:** `src/dopamine/dopamine.hpp`, `src/dopamine/dopamine.cpp` (novos), `src/synapse/synapse.hpp`, `src/synapse/synapse.cpp`, `CMakeLists.txt` (uma linha em `child_core`), `tests/tests.cpp` (um caso novo, `three-factor plasticity`)
**Depende de:** cerebro/SIN-01 (integrada pela master/0008)
**Verificação:**
- **Risco vital:** o terceiro fator decide o que a rede guarda. Uma marca que não expira, ou um peso que muda sem dopamina, faria o APR-01 aprender ruído.
- **Lacuna:** nenhuma proteção.
- **Um caso `three-factor plasticity`** cobre os dois aceites do DOP-01:
  - pré→pós +5 ms com dopamina a 0,2 s e a 1 s altera o peso, no sentido do STDP e menos com o atraso maior;
  - pós→pré com dopamina reduz o peso;
  - o mesmo par sem dopamina por 10 s não muda o peso;
  - com dopamina a 6 s (depois de 5·τ_e), o peso também não muda;
  - a igualdade é exata, porque a marca expira.
- **O caso `stdp synapse` continua passando sem edição.** Ciclo vermelho → verde.

**Instrução para o executor:**
> Spec master/0009 — Task 01: implementar o DOP-01 do contrato `cerebro@2`.
> - **Dopamina:** em `src/dopamine/` (arquivo principal `dopamine`), um sinal global com liberações pontuais e decaimento τ_d = 0,2 s e basal zero.
> - **Sinapse:** um caminho de três fatores. A mudança proposta por `on_pre`/`on_post` vai para uma marca de elegibilidade com τ_e = 1 s, que expira (vira zero) após 5·τ_e. Um método de reforço recebe o nível de dopamina e o passo de tempo e aplica `e·d·dt` ao peso, com o corte em [0, w_max]. Manter `apply` e o comportamento do SIN-01.
> - **Independência:** a sinapse não inclui `dopamine/`, e a dopamina não inclui `synapse/`.
> - **Teste antes:** em `tests/tests.cpp`, escrever o caso `three-factor plasticity` com os cenários da verificação (dopamina a 0,2 s e 1 s; sem dopamina; dopamina a 6 s; par invertido), vê-lo falhar e então implementar.
> - **Limites:** float32, sem `<random>`, sem avisos; não tocar no MLP, no neurônio nem no CLI.
> - **Ao terminar:** rodar `ctest` e registrar evidência do DOP-01 com `record-evidence.py`.
> - **Relatório:** fontes com linhas, a evidência, os valores impressos e as escolhas técnicas. Não escrever estado, specs nem índices.

## Validação e integração
- **Critérios da entrega:**
  - DOP-01 implementado, com evidência corrente;
  - `ctest` verde, com `stdp synapse` inalterado, `brain_layout` verificando `dopamine/` e `reference weights` com o hash inalterado;
  - build do zero sem avisos.
- **Gate:** `validate-lifecycle.py . --ready cerebro/DOP-01`.
- **Consolidação pelo coordenador:**
  - `.centaur/state/cerebro.json` (DOP-01, PAR-01 com τ_e e τ_d);
  - refazer as evidências que dependem de `tests/tests.cpp`, `CMakeLists.txt` ou `synapse.*`;
  - registro em `.centaur/implements/`;
  - linha "Onde estamos" do README.
- **Integração:** branch e PR, quando autorizado.

## Diferença entre plano e resultado
- Implementação: [0008](../../implements/0008/README.md). Seguiu o plano: τ_e 1 s, expiração em 5 s, τ_d 0,2 s, sem basal, `dw = e·d·dt` por Euler.
- Escolha interna: a liberação de dopamina é medida pela integral do pulso, para a escala do reforço não depender de τ_d.
- Mudaram `tests/tests.cpp`, `CMakeLists.txt`, `synapse.*` e o README, por isso as evidências afetadas foram refeitas.
- Na branch `dopamine-third-factor`, com PR; falta o merge.

## Checklist de conclusão
- [x] Task 01 — Marca de elegibilidade e sinal de dopamina

## Execução
/centaur-driven-run master/0009

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
