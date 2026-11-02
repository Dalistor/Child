# [0008] Dopamina como terceiro fator

**Data:** 2026-11-02
**Status:** Concluído
**Modo:** TDD
**Spec:** `.centaur/specs/0009/` — Task 01

## Solicitação
"Crie a spec da dopamina" e "pode executar".

## O que foi feito
- `src/dopamine/`: sinal global em pulsos, que decai com τ_d = 0,2 s e não tem basal. Uma liberação de valor `amount` integra `amount` ao longo do tempo.
- `src/synapse/`: marca de elegibilidade com τ_e = 1 s, que expira (vale exatamente zero) 5 s depois da última marca. `tag()` guarda a mudança proposta pelo STDP; `reinforce(dopamina, t, dt)` aplica `e·d·dt` com o corte em [0, w_max]. O caminho direto (`apply`) do SIN-01 continua igual.
- Caso `three-factor plasticity`.
- README, `AGENTS.md` e `architecture.md` atualizados.

## Arquivos criados
- `src/dopamine/dopamine.{hpp,cpp}`

## Arquivos modificados
- `src/synapse/synapse.{hpp,cpp}`, `CMakeLists.txt`, `tests/tests.cpp`, `README.md`, `AGENTS.md`, `.centaur/system/architecture.md`

## Decisões técnicas
- **Unidade da dopamina:** a liberação é medida pela integral do pulso. Assim, uma unidade de dopamina transforma uma unidade de marca em peso, independentemente de τ_d.
- **Expiração em 5·τ_e:** sem ela a exponencial nunca chega a zero, e o segundo aceite do DOP-01 não seria observável.
- **Sem dopamina basal:** com basal, toda marca viraria peso.
- **Integração:** a dopamina é integrada por Euler com o passo da simulação, conforme previsto na spec.

## Como validar
`ctest --test-dir build`: o caso `three-factor plasticity` imprime a mudança de peso por atraso da dopamina.

## Contrato e realização
`cerebro@2` DOP-01 implementada e verificada (ev-ea5a45026dec49e79ffeb04b7f6dfb72). PAR-01 ganhou τ_e e τ_d (continua parcial). As evidências afetadas foram refeitas. Entrega local, na branch `dopamine-third-factor`.

## Resultado da validação
Ciclo vermelho → verde. `ctest` 3/3, build sem avisos.
- Dopamina a 0,2 s: +0,00266; a 1 s: +0,00119.
- Dopamina a 6 s, ou nenhuma: exatamente 0.
- Par invertido: −0,00279.
- `stdp synapse` e `reference weights` não mudaram.
