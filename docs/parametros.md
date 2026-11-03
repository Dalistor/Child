# Parâmetros: faixas publicadas

Levantamento de 2026-11-02. Os valores abaixo saíram da leitura das fontes citadas, não de resumos de busca. **Lidos na íntegra (PDF ou texto completo):** Brette & Gerstner 2005; Naud et al. 2008; Bi & Poo 1998; Song, Miller & Abbott 2000; Frémaux & Gerstner 2016; Turrigiano 2008; Zenke, Hennequin & Gerstner 2013; Masquelier, Guyonneau & Thorpe 2008; Schultz, Dayan & Montague 1997. **Lido em parte:** Vogels et al. 2011 (texto principal da versão aceita; os Métodos ficam no material suplementar, que não consegui acessar, e por isso os números vêm do código dos autores no ModelDB); Izhikevich 2007 (artigo pago; os números vêm da patente US 8,103,602 do mesmo autor, que repete o método, e do resumo do CNS 2007). **Só o resumo:** Turrigiano et al. 1998. **Sem acesso:** van Rossum, Bi & Turrigiano 2000 (PMC e editora bloquearam), Brunel 2000 e Abbott & Nelson 2000 (não lidos). Quando duas fontes divergem, as duas faixas aparecem lado a lado, sem escolha entre elas.

## 1. Neurônio AdEx

Equações: `C dV/dt = −g_L(V−E_L) + g_L Δ_T exp((V−V_T)/Δ_T) − w + I`; `τ_w dw/dt = a(V−E_L) − w`; quando V passa de V_peak: `V ← V_reset`, `w ← w + b`.

| parâmetro | unidade | faixa | valor típico | fonte | trecho / onde |
|---|---|---|---|---|---|
| C | pF | 59–281 | 281 (BG05); 100–200 (Naud) | [Brette & Gerstner 2005](https://doi.org/10.1152/jn.00686.2005), [Naud et al. 2008](https://doi.org/10.1007/s00422-008-0264-7) | BG05, Tabela 1: "C (membrane capacitance) 281 pF". Naud, Tabela 1: de 59 (cNA) a 200 pF |
| g_L | nS | 1,7–30 | 30 (BG05); 10–12 (Naud) | idem | BG05, Tabela 1: "gL 30 nS". Naud, Tabela 1: de 1,7 a 18 nS |
| E_L | mV | −70,6 a −58 | −70,6 (BG05); −70 (Naud) | idem | BG05, Tabela 1: "EL −70.6 mV"; Naud, Tabela 1 |
| V_T | mV | −56 a −42 | −50,4 (BG05); −50 (Naud) | idem | BG05, Métodos: "We therefore chose VT = −50.4 mV and ΔT = 2 mV as the optimal values" |
| Δ_T | mV | 0,8–5,5 | 2 | idem | BG05: a variância do limiar efetivo é mínima com "slope factor 2 mV" (legenda da Fig. 1D) |
| a | nS | −11 a 4 | 4 (BG05); 2 (Naud, tônico e adaptativo) | idem | BG05, Tabela 1: "a 4 nS". Naud: a < 0 gera disparo atrasado ou irregular (Fig. 4e–h) |
| τ_w | ms | 16–300 | 144 (BG05) | idem | BG05: "163, 144, and 120 ms at 5, 10, and 20 Hz… we took b = 80.5 pA and τw = 144 ms" |
| b | pA | 0–120 | 80,5 (BG05); 60 (Naud, adaptativo) | idem | BG05, Tabela 1: "b 0.0805 nA". Naud: b = 0 dá disparo tônico (Fig. 4a) |
| V_reset | mV | −58 a −46 (Naud); = E_L (BG05) | −70,6 (BG05); −58 (Naud) | idem | BG05: "restarted from a reset value Vr, with Vr = EL". Naud: V_reset alto (> V_T) produz rajadas |
| V_peak (corte) | mV | 0 a 20 | 20 (BG05); 0 (Naud) | idem | BG05: "Vpeak = 20 mV, but the exact value is not critical because it only shifts spike times by a fraction of millisecond". Naud: "if V > 0 mV then" |

Conjuntos de Naud et al. 2008 (Tabela 1, mesma ordem de colunas: C, g_L, E_L, V_T, Δ_T, a, τ_w, b, V_reset, I), úteis para testar padrões de disparo:

| padrão (Fig. 4) | C pF | g_L nS | E_L mV | V_T mV | Δ_T mV | a nS | τ_w ms | b pA | V_reset mV | I pA |
|---|---|---|---|---|---|---|---|---|---|---|
| a tônico | 200 | 10 | −70 | −50 | 2 | 2 | 30 | 0 | −58 | 500 |
| b adaptativo | 200 | 12 | −70 | −50 | 2 | 2 | 300 | 60 | −58 | 500 |
| c rajada inicial | 130 | 18 | −58 | −50 | 2 | 4 | 150 | 120 | −50 | 400 |
| d rajadas regulares | 200 | 10 | −58 | −50 | 2 | 2 | 120 | 100 | −46 | 210 |
| e atrasado e acelerando | 200 | 12 | −70 | −50 | 2 | −10 | 300 | 0 | −58 | 300 |
| f rajada regular atrasada | 200 | 12 | −70 | −50 | 2 | −6 | 300 | 0 | −58 | 110 |
| g transitório | 100 | 10 | −65 | −50 | 2 | −10 | 90 | 30 | −47 | 350 |
| h irregular | 100 | 12 | −60 | −50 | 2 | −11 | 130 | 30 | −48 | 160 |

Para o NEU-01 (adaptação sob corrente constante), o conjunto "b adaptativo" ou os valores de BG05 servem.

## 2. Sinapse com STDP

| parâmetro | unidade | faixa | valor típico | fonte | trecho / onde |
|---|---|---|---|---|---|
| janela LTP / LTD (experimental) | ms | ~20 de cada lado; nada fora de ~40 | — | [Bi & Poo 1998](https://doi.org/10.1523/JNEUROSCI.18-24-10464.1998) | Resultados, "The critical window", Fig. 7: "Potentiation… within a time window of 20 msec after… depression… within a window of 20 msec before… outside the 40 msec window, synaptic modification was essentially absent. A narrow transition range of ~5 msec" |
| magnitude (experimental) | % | LTP +48,4 ± 9,9 (n=14); LTD −18,0 ± 3,2 (n=12) | — | Bi & Poo 1998 | Figs. 2C e 4C; protocolo de 60 pulsos a 1 Hz; só vale para EPSC inicial < 500 pA (sinapses fortes não potenciam) |
| τ+ | ms | 16,8–20 | 20 (Song); 16,8 (Masquelier) | [Song et al. 2000](https://doi.org/10.1038/78829), [Masquelier et al. 2008](https://doi.org/10.1371/journal.pone.0001377) | Song: "Experimental results suggest a value for τ+ in the range of tens of milliseconds… we use τ+ = 20 ms". Masquelier, Métodos: "τ+ = 16.8 ms and τ− = 33.7 ms" (ajuste de Bi & Poo 2001, que não li) |
| τ− | ms | 20–33,7 | 20 (Song); 33,7 (Masquelier) | idem | Song: "we use τ- = τ+ = 20 ms" |
| A+ | fração de w_max por par | 0,005–0,03125 | 0,005 | idem | Song: "A+ = 0.005 thus corresponds to a change of 0.5% of the maximum synaptic strength per spike pair" (0,02 na Fig. 2f). Masquelier: "a+ = 0.03125" |
| A−/A+ | — | 0,85–1,05 | 1,05 (Song) | idem | Song: "we use A-/A+ = 1.05". Masquelier: "a− = 0.85·a+" |
| A−τ−/A+τ+ (LTD domina) | — | 1,05–1,70 | ligeiramente > 1 | Song; Masquelier; Izhikevich (patente) | Song: "A negative integral of F requires A-τ- > A+τ+". Masquelier: 0,85·33,7/16,8 ≈ 1,70. Izhikevich: "The LTD area… is 50% greater than the LTP area" (≈ 1,5) |
| g_max | condutância em unidades de g_leak | 0,015–0,035 | 0,015 (≈ 150 pS com 100 MΩ) | Song et al. 2000 | Métodos: "ḡmax = 0.015 (except for Fig. 4, where ḡmax = 0.02 and Fig. 2d, where ḡmax = 0.035). For a 100 MΩ input resistance, ḡmax = 0.015 corresponds to a peak synaptic conductance of 150 pS" |
| limites do peso | — | [0, g_max], corte rígido | — | Song; Masquelier | Song: "If this modification rule would push the peak synaptic conductance beyond the allowed range 0 ≤ ḡa ≤ ḡmax, ḡa is set to the appropriate limiting value". Masquelier: pesos "clipped to [0,1]" |
| neurônio de Song (LIF) | ms, mV | — | τ_m 20; V_rest −70; limiar −54; reset −60; E_ex 0; E_in −70; τ_ex = τ_in 5 | Song et al. 2000 | Métodos; 1000 sinapses excitatórias e 200 inibitórias, entradas de 10–40 Hz |

**Valores no código: transmissão** (`ReceptorParams` em `src/neuron/neuron.hpp`, desde a master/0010). Um disparo abre uma condutância no neurônio alvo, que decai exponencialmente: `I_syn = g_E (E_E − V) + g_I (E_I − V)`.

| parâmetro | código | faixa / fonte |
|---|---|---|
| E_E | 0 mV | Vogels 2011 (V_E = 0 mV); Song 2000 (E_ex 0) |
| E_I | −80 mV | Vogels 2011 (V_I = −80 mV); Song 2000 usa −70 |
| τ_E | 5 ms | Vogels 2011 (5 ms); Song 2000 (5 ms) |
| τ_I | 10 ms | Vogels 2011 (10 ms); Song 2000 usa 5 |
| condutância de pico de uma sinapse excitatória com peso máximo | 1 nS (teste `homeostatic scaling`) | g_max de Song, 0,015–0,035·g_L, dá 0,45–1,05 nS com o g_L de 30 nS do BG05 |

## 3. Elegibilidade e dopamina (três fatores)

| parâmetro | unidade | faixa | valor típico | fonte | trecho / onde |
|---|---|---|---|---|---|
| τ_e (marca de elegibilidade) | s | centenas de ms a ~1 s (modelos); segundos (experimentos citados) | 1 (Izhikevich) | [Frémaux & Gerstner 2016](https://doi.org/10.3389/fncir.2015.00085); Izhikevich 2007 ([artigo](https://doi.org/10.1093/cercor/bhl152), [patente US 8,103,602](https://patents.google.com/patent/US8103602B2/en)) | Frémaux: "τe… determines the maximal interval between the pre-post coincidences and the reward"; "a few hundred milliseconds for eligibility traces in reinforcement learning". Patente: "decays exponentially to c=0 with the time constant τc=1 second" (**não confirmado no artigo**) |
| τ_d (decaimento da dopamina) | s | — | 0,2 | Izhikevich (patente) | "In the present simulations τd=0.2 second" (**não confirmado no artigo**) |
| atraso da recompensa tolerado | s | 1–3 | — | Izhikevich (patente) | "with a random delay between 1 and 3 seconds" |
| dopamina tônica (basal) | nM | — | 2 | Izhikevich (patente) | "the baseline (tonic) concentration of DA is 2 nM" |
| janela experimental (agonista D1 e LTP) | s | perde o efeito se chega 15–25 s depois | — | [Izhikevich, resumo CNS 2007](https://doi.org/10.1186/1471-2202-8-S2-S15) | "the effect disappears if the agonist arrives at the synapses 15–25 seconds after the tetanus" |
| marca sináptica (tagging), para comparação | h | ~1 | — | Frémaux & Gerstner 2016 | "for tagging the decay of traces occurs on the time scale of ~1 hr" (outro mecanismo; não deve ser confundido com τ_e) |

Equação de referência (Frémaux, Eq. 6): `de/dt = −e/τ_e + STDP(pre, post)`; `dw/dt = e · M(t)`.

## 4. Homeostase e escala sináptica

| parâmetro | unidade | faixa | valor típico | fonte | trecho / onde |
|---|---|---|---|---|---|
| escala de tempo biológica | h | 4 h a 2 dias | ~24–48 h | [Turrigiano et al. 1998](https://doi.org/10.1038/36103) (só o resumo), [Turrigiano 2008](https://doi.org/10.1016/j.cell.2008.10.008) | 1998: "over a 48-hour period mEPSC amplitudes decreased and firing rates returned to close to control values". 2008: "over a time scale of many hours"; "even the earliest phases of synaptic scaling (within the first 4 hr) are dependent on transcription" |
| integração do sensor de atividade | min a h | — | — | Turrigiano 2008 | "integrate this measure over a time step that is long (minutes to hours) relative to the time scale of information transfer (milliseconds to minutes)" |
| natureza | — | multiplicativa | — | Turrigiano 1998 e 2008 | 1998: "affected each synapse in proportion to its initial strength". 2008: "scaling up or down proportionally allows neurons to stabilize firing without changing the relative strength of synaptic inputs" |
| τ_hom crítico (modelo) | s | ~25 s (rede) a ~3 min (campo médio) | abaixo de τ_crit | [Zenke et al. 2013](https://doi.org/10.1371/journal.pcbi.1003330) | "τ must be shorter than some critical value τcrit ≈ 25 s"; "for a background rate of 3 Hz and the learning rate η = 1 we find a critical timescale of τcrit ≈ 3 min (simulations: 166.5 ± 0.5 s, mean field model: 170.6 s)"; "For τ < 3 s the network has a tendency to fall silent" |
| taxa-alvo κ | Hz | — | 3 | Zenke et al. 2013 | "homeostatic target rate of (κ = 3 Hz)"; a rede é considerada instável acima de 60 Hz |
| constante de escala em van Rossum 2000 | — | — | — | [van Rossum et al. 2000](https://doi.org/10.1523/JNEUROSCI.20-23-08812.2000) | **Não confirmado: o texto não estava acessível.** |

**Tensão registrada.** Na biologia, a escala sináptica leva horas ou dias (Turrigiano). Nos modelos com STDP, a homeostase precisa detectar mudanças de taxa em segundos ou minutos, senão a rede dispara sem controle (Zenke 2013: "homeostasis has to detect rate changes on a timescale of seconds to minutes to keep the activity stable"). O próprio Zenke registra que existem formas rápidas de escala, algumas na faixa de minutos, mas a questão continua em aberto. A regra HOM-01 ("pelo menos uma ordem de grandeza acima da janela do STDP") cabe nessa faixa: com janela de 20 ms, basta τ_hom ≥ 0,2 s.

**Valores no código: homeostase** (`HomeostasisParams` em `src/synapse/homeostasis.hpp`, desde a master/0010). Um sensor de taxa por neurônio filtra os disparos com τ_hom. Um ganho comum multiplica todos os pesos de entrada (peso efetivo = peso × ganho), o que preserva a proporção entre eles: `ganho ← ganho · (1 + η·dt·(1 − taxa/alvo))`.

| parâmetro | código | faixa / fonte |
|---|---|---|
| taxa-alvo | 5 Hz | aprovada no `cerebro@2` (HOM-01); entre κ = 3 Hz (Zenke) e ρ0 ≈ 3–6 Hz (Vogels) |
| τ_hom | 5 s | aprovada no `cerebro@2`, faixa 1–20 s; abaixo de τ_crit ≈ 25 s (Zenke 2013) |
| η | 10⁻⁴ por ms (escala de tempo 1/η = 10 s) | **sem valor publicado na mesma forma.** Escolhido para a escala agir em segundos, abaixo de τ_crit (Zenke 2013), e ≥ 10 vezes a janela do STDP. No teste, depois de a entrada dobrar, a taxa em 20 s volta a 4–6 Hz antes dos 50 s, com um mergulho a ~2 Hz no caminho |

## 5. Inibição

| parâmetro | unidade | faixa | valor típico | fonte | trecho / onde |
|---|---|---|---|---|---|
| regra inibitória | — | — | Δw = η[pre×post − ρ0×pre] | [Vogels et al. 2011](https://doi.org/10.1126/science.1211095) | texto principal: "ρ0 is a constant that acts as a target rate for the postsynaptic" neuron; "the postsynaptic firing rate is determined mainly by the depression factor ρ0" |
| ρ0 (taxa-alvo) | Hz | ~3–6 | 3 (rede) | Vogels 2011, [código ModelDB 143751](https://github.com/ModelDBRepository/143751) | `isp_brian.py`: `alpha = 3*Hz*tau_stdp*2  # Target rate parameter`. No neurônio isolado (`inhib_plasticity.m`), α = 0,25 com τ = 20 ms dá ρ0 = α/(2τ) ≈ 6 Hz (conta minha). **O suplemento não foi lido.** |
| τ_STDP inibitório | ms | — | 20 | código ModelDB | `tau_stdp = 20*ms`; `tauPlasticity=20` |
| η (taxa de aprendizado) | — | 1e−3 a 1e−2 | 1e−2 (rede) | código ModelDB | `eta = 1e-2` (rede); `eta=0.001` (neurônio isolado) |
| E_I (reversão GABA) | mV | — | −80 | Vogels 2011 | texto principal: "reversal potentials VE = 0 mV and VI = −80 mV" |
| τ_E / τ_I (sinápticas) | ms | — | 5 / 10 | Vogels 2011 | "time constants τE = 5 ms, and τI = 10 ms" |
| condutâncias (rede) | nS | E 0,3; I→I 3; E←I de ~0 até g_max = 100 | — | código ModelDB | `w = 1.*nS`; `con_e weight=0.3`; `con_ii weight=3`; `con_ie weight=1e-10`; `gmax = 100` |
| neurônio da rede | pF, nS, mV, ms | — | C 200; g_L 10; E_L −60; V_T −50; reset −60; refratário 5 | código ModelDB | `memc = 200.0*pfarad`, `gl = 10.0*nsiemens`, `el = -60*mV`, `vt = -50.*mV`, `refractory=5*ms` |
| conectividade | — | 2% (Vogels); 10% (Izhikevich) | — | código ModelDB; Izhikevich (patente) | `epsilon = 0.02`; "randomly connected with 10% probability" |
| proporção E/I | — | 80/20 (4:1) | 80/20 | código ModelDB; Izhikevich (patente); Zenke 2013 | `NE = 8000`, `NI = NE/4`; "80% excitatory… and 20% inhibitory"; Zenke: "20000 excitatory and 5000 inhibitory units". Brunel 2000 **não lido** |
| entrada do neurônio isolado | — | 8 grupos × (100 E + 25 I) | — | Vogels 2011 | texto principal, Fig. 1B; sinal com τ ≈ 50 ms |

**Valores no código: inibição** (`src/interneuron/interneuron.hpp`, desde a master/0010).

| parâmetro | código | faixa / fonte |
|---|---|---|
| neurônio inibitório genérico | conjunto "a tônico" de Naud 2008: C 200 pF, g_L 10 nS, E_L −70, V_T −50, Δ_T 2, a 2 nS, τ_w 30 ms, b 0, V_reset −58 | §1 (Naud, Tabela 1); b = 0 dá disparo sem adaptação, como uma célula de disparo rápido |
| condutância por disparo na sinapse inibitória (fixa) | 3 nS | Vogels: I→I 3 nS, E←I de ~0 a 100 nS |

Configuração do teste `inhibition`, que não é um padrão do código:
- 200 neurônios excitatórios (BG05) e 50 inibitórios, ou seja 4:1;
- E→E com 10% de conectividade e STDP. O pico é de 12 nS por sinapse com peso máximo, e o peso inicial é 0,3;
- E→I com 50% e 1 nS fixo; I→E denso, com 3 nS;
- fundo de 200 entradas a 20 Hz, agregadas num processo de 4000 Hz, com 0,6 nS por disparo.

O pico recorrente fica acima da faixa de Song porque a rede é pequena: com ~20 entradas recorrentes por neurônio, em vez de milhares, cada uma precisa pesar mais para a recorrência importar. A conectividade densa do interneurônio para as células vizinhas é a das células em cesto. Os 50% de E→I também ficam acima dos 2–10% de rede grande, pelo mesmo motivo.

## Confirmação das afirmações da síntese

| afirmação (docs/sintese.md) | veredito | o que a fonte diz |
|---|---|---|
| (a) AdEx prevê com boa precisão o tempo dos disparos do Hodgkin-Huxley | **CONFIRMA, com ressalva** | Brette & Gerstner 2005: "predicts correctly the timing of 96% of the spikes (±2 ms) of the detailed model"; Γ = 0,96 (3% de disparos a mais e 4% a menos). Ressalvas: o modelo de referência é do *tipo* Hodgkin-Huxley (pirâmide regular de um compartimento, McCormick 1993 / Destexhe 1998, com I_M, I_Na e I_K), não o axônio de lula de 1952. O teste usa condutâncias flutuantes. Sem adaptação (b = 0), Γ cai para 0,67. |
| (b) STDP sem estabilizador diverge | **CORRIGE em parte** | Song 2000: "STDP alone can lead to stable distributions of synaptic conductances, subject only to a limit on the strengths of individual synapses" (estável por 100 h simuladas), desde que A−τ− > A+τ+. Os pesos ficam bimodais (vão para 0 ou g_max). Song também diz que o STDP é "destabilizing at individual synapses" e "cannot rescue" um neurônio que parou de disparar. Em rede recorrente (STDP triplet), sem homeostase rápida a atividade explode (Zenke 2013). Formulação mais exata: o STDP aditivo precisa de limite por sinapse e de LTD dominante; mesmo assim, em rede, precisa de homeostase ou inibição. Abbott & Nelson 2000 e van Rossum 2000 não foram lidos. |
| (c) A homeostase age em horas ou dias, bem mais devagar que o STDP | **CONFIRMA para a biologia; CORRIGE para o modelo** | Turrigiano 1998/2008: dezenas de horas (48 h), fase inicial em ~4 h. Zenke 2013: num modelo, a homeostase precisa reagir em segundos ou minutos (τ_crit ≈ 25 s a 3 min). Formas rápidas de escala existem, algumas em minutos. |
| (d) A dopamina fásica acompanha o erro de predição de recompensa | **CONFIRMA** | [Schultz, Dayan & Montague 1997](https://doi.org/10.1126/science.275.5306.1593), Fig. 1 e texto: a dopamina sobe com recompensa melhor que a prevista, não muda com recompensa prevista e cai "exactly at the time when the reward would have occurred" quando a recompensa falta. |

## Proposta de limiares

Proposta, não decisão. Refere-se a `.centaur/contracts/cerebro/v002.json` (APR-01 e HOM-01). A configuração imita a de [Masquelier et al. 2008](https://doi.org/10.1371/journal.pone.0001377), reduzida para um teste determinístico (dt fixo, por exemplo 0,1 ms, e semente fixa).

**Referência de Masquelier 2008.** 2000 aferentes a 64 Hz de média, mais 10 Hz de ruído de fundo. Metade deles repete um padrão de 50 ms com 1 ms de jitter, em 1/4 do tempo, e o padrão tem a mesma densidade de disparos do ruído (não se distingue pela taxa). A seletividade surge "after only about 70 pattern presentations and 700 discharges" (13,5 s). A convergência chega com ~2000 disparos. Em 450 s há ≈ 2250 apresentações (450 × 0,25 / 0,05). Critério de sucesso: "latency inferior to 10 ms, a hit rate superior to 98% and no false alarms", atingido em 96% de 100 sementes. Abaixo de 15% de frequência relativa do padrão, o desempenho cai.

| limiar | proposta | justificativa |
|---|---|---|
| APR-01 · N (apresentações antes de medir) | **500** (50 ms cada, 1/4 do tempo, ≈ 100 s simulados) | ≈ 7 vezes as ~70 apresentações em que a seletividade aparece em Masquelier, com folga para AdEx e homeostase. Fica abaixo das 2250 do experimento completo, o que mantém o teste barato |
| APR-01 · R (taxa no padrão / taxa no ruído) | **≥ 5**, medida nas últimas 100 apresentações (disparos/s dentro das janelas de padrão ÷ disparos/s fora delas) | Masquelier chega a 99% de acerto e zero falso alarme, ou seja, razão muito alta. R = 5 deixa margem para um neurônio mais simples. Como o padrão tem a mesma densidade do ruído, com a plasticidade desligada a razão esperada fica perto de 1. Isso garante que o critério "sem plasticidade, abaixo de R" separa bem os dois casos |
| APR-01 · T (treino longo) | **5000 apresentações** (≈ 1000 s simulados) | Song 2000 coletou dados "only after 1000 seconds of simulated time" e viu convergência em ~100 s. Isso equivale a ≈ 2 vezes o experimento de Masquelier e a ≥ 100 τ_hom com a proposta abaixo |
| HOM-01 · taxa-alvo | **5 Hz** | Fica entre κ = 3 Hz (Zenke) e ρ0 ≈ 3–6 Hz (Vogels). Também é a taxa de um detector que dispara uma vez por padrão quando o padrão ocorre 5 vezes por segundo (1/4 do tempo, 50 ms), de modo que APR-01 e HOM-01 não brigam entre si |
| HOM-01 · tolerância | **±20%** (4–6 Hz), média numa janela de 20 s | Com ~100 disparos na janela, a flutuação de contagem do tipo Poisson fica em ~10%. ±20% equivale a ~2σ e é robusto entre sementes. Zenke registra que as taxas finais ficam "slightly above" o alvo |
| HOM-01 · τ_hom (auxiliar) | **5 s** (faixa aceitável de 1 a 20 s); período de adaptação de 10 τ_hom = 50 s | Fica ≥ 10 vezes a janela do STDP (20 ms), como pede a HOM-01, e abaixo de τ_crit ≈ 25 s (Zenke). É bem mais rápido que a biologia (horas); a tensão descrita na seção 4 fica registrada aqui como escolha consciente |
