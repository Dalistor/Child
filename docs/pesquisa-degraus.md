# Pesquisa: dendritos, cerebelo, hipocampo e neurogênese

Levantamento de 2026-11-07 para os degraus 2 a 5 e para o crescimento de neurônios. Cada fonte indica o nível de acesso: **íntegra** (texto do artigo lido), **resumo** (só o abstract) ou **secundária** (citada por outros). As páginas HTML do PMC estavam bloqueadas; o texto veio do Europe PMC, de PDFs dos autores e do arXiv.

## 1. Pico de cálcio dendrítico (dCaAP) e XOR num neurônio

[Gidon et al. 2020, Science 367:83](https://doi.org/10.1126/science.aax6239) — **íntegra** (manuscrito aceito, [zenodo 3758325](https://zenodo.org/records/3758325)); modelo na [ModelDB 260178](https://modeldb.science/260178), código não lido.

| Grandeza | Valor | Trecho |
|---|---|---|
| Ativação | não monotônica | "amplitudes were maximal for threshold-level stimuli but dampened for stronger stimuli" |
| Amplitude | 43,8 ± 13,8 mV (13–67) | "range between 13.0 and 67.0 mV; n = 32" |
| Largura | 4,4 ± 1,4 ms (2,6–8,0) | "4.4 ± 1.4 ms, range between 2.6 and 8.0 ms" |
| Decaimento da ativação acima do limiar | τ ≈ 0,39 (em unidades da reobase) | "sharply tuned (highly selective) to a particular input strength" |
| Atraso até o disparo somático | 21,6–116,9 ms (média 53,8) | 17 de 37 células acopladas |
| Modelo | limiar −36 mV com 220 pA; τ_dCaAP = 0,3 | duas vias X e Y de 25 sinapses AMPA+NMDA cada |

XOR: X sozinho ou Y sozinho produz o dCaAP; X e Y juntos passam da reobase e o amplitude cai, e o dendrito se comporta como XOR. A inibição recupera a amplitude.

## 2. Platôs NMDA e segmentos dendríticos

- [Antic et al. 2010, J Neurosci Res 88:2991](https://doi.org/10.1002/jnr.22444) — **resumo**: "Synchronous activation of 10-50 neighboring glutamatergic synapses triggers… NMDA spike/plateau… (40-50 mV) and an extraordinary duration (up to several hundred milliseconds)".
- [Schiller et al. 2000, Nature 404:285](https://doi.org/10.1038/35005094) — **resumo**: no soma, 5,9 ± 1,5 mV de pico e 64,4 ± 19,8 ms de meia-largura.
- [Hawkins & Ahmad 2016, Front Neural Circuits 10:23](https://doi.org/10.3389/fncir.2016.00023) — **íntegra**: "the coincident activation of 8–20 synapses" gera o pico NMDA; 20–40 sinapses por segmento; limiar θ ≈ 15 (robusto com ≥ 12 de limiar e 20 sinapses por padrão); 2% das minicolunas ativas; 32 células por minicoluna; permanência em [0, 1] com conexão a partir de 0,5 e incrementos menores que 0,1. Duração do platô não informada.

## 3. Cerebelo (Marr–Albus–Ito)

- [Billings et al. 2014, Neuron 83:960](https://doi.org/10.1016/j.neuron.2014.07.020) — **íntegra**: cada grânulo recebe "approximately four excitatory MF inputs"; "two to seven… with four inputs performing particularly well"; limiar em 75% das entradas; razão local de rosetas para grânulos de 1:2,9.
- [Litwin-Kumar et al. 2017, Neuron 93:1153](https://doi.org/10.1016/j.neuron.2017.01.030) — **íntegra**: razão de expansão ≈ 30 (N = 7.000 fibras musgosas, M = 209.000 grânulos por Purkinje); ótimo em K = 4; nível de codificação f = 0,01 no cerebelo; acima de 10–50 de expansão a dimensão não cresce para f = 0,1.
- [Ito & Kano 1982, J Physiol](https://doi.org/10.1113/jphysiol.1982.sp014103) — **resumo**: LTD na sinapse fibra paralela → Purkinje, específica das sinapses ativas junto com a fibra trepadeira. [Ito 2001, Physiol Rev 81:1143](https://doi.org/10.1152/physrev.2001.81.3.1143) — **resumo**: a LTD elimina "those synaptic connections associated with errors".
- **Taxa de aprendizado:** nenhuma fonte dá valor biológico para a regra delta; é hiperparâmetro livre.

## 4. Hipocampo e sistemas complementares

- [McClelland, McNaughton & O'Reilly 1995, Psych Rev 102:419](https://doi.org/10.1037/0033-295x.102.3.419) — **resumo**: a estrutura surge "if learning of each item is gradual and interleaved"; o hipocampo "permits rapid learning of new items without disrupting this structure".
- [Kumaran, Hassabis & McClelland 2016, TICS](https://doi.org/10.1016/j.tics.2016.05.004) — **resumo**: o replay pondera as estatísticas da experiência.
- Giro denteado esparso: [Jung & McNaughton 1993](https://doi.org/10.1002/hipo.450030209) — **resumo**: "88% of cells… have mean rates below 0.5 Hz". O número de ~2% de grânulos ativos (Ramirez-Amaya et al. 2013) foi visto só em trechos de busca (**secundária**).
- Compressão do replay: [Lee & Wilson 2002, Neuron](https://doi.org/10.1016/S0896-6273(02)01096-6) — **resumo**: surtos de ~100 ms, "compressing the behavioral sequence in time by approximately 20-fold". [Buzsáki 2015, Hippocampus 25:1073](https://doi.org/10.1002/hipo.22488) — **íntegra**: "10 to 20 times faster"; ondas agudas de 40–100 ms; ripples de 110–200 Hz; recorrência de CA3 com 5–10% dos alvos, substrato da autoassociação.
- Número de repetições por noite: sem valor canônico encontrado.

## 5. Neurogênese e unidades criadas sob demanda

- [Ge et al. 2007, Neuron 54:559](https://doi.org/10.1016/j.neuron.2007.05.002) — **resumo**: período crítico de 1 a 1,5 mês da idade da célula, com "enhanced long-term potentiation with increased potentiation amplitude and decreased induction threshold".
- [Aimone, Wiles & Gage 2009, Neuron 61:187](https://doi.org/10.1016/j.neuron.2008.11.026) e [Aimone, Deng & Gage 2011, Neuron 70:589](https://doi.org/10.1016/j.neuron.2011.05.010) — **resumo**: neurônios jovens são de sintonia larga e os maduros, específicos ("memory resolution hypothesis").
- [Spalding et al. 2013, Cell 153:1219](https://doi.org/10.1016/j.cell.2013.05.002) — **resumo**: "700 new neurons are added in each hippocampus per day", renovação anual de 1,75% da fração que se renova.
- Controvérsia: [Sorrells et al. 2018](https://doi.org/10.1038/nature25975) não encontram neurônios jovens em adultos; [Boldrini et al. 2018](https://doi.org/10.1016/j.stem.2018.03.015) encontram "thousands of immature neurons". Ambos **resumo**. O Child usa a neurogênese como mecanismo de engenharia inspirado no giro denteado, não como afirmação sobre o cérebro adulto humano.
- [Draelos et al. 2017, IJCNN (arXiv 1612.03770)](https://arxiv.org/abs/1612.03770) — **íntegra**: amostras com erro acima de um limiar contam como "outliers"; ao juntar um número crítico, novos nós são adicionados (limites "MaxNodes" e "MaxOutliers"); primeiro treinam só os novos, depois o nível é estabilizado por replay.
- [Fritzke 1995, NIPS 7](https://papers.nips.cc/paper/1994/hash/d56b9fc4b0f1be8871f5e1c40c0067e7-Abstract.html) — **íntegra**: a cada λ = 100 entradas, uma unidade é inserida perto da que acumulou mais erro.
- ART ([Carpenter & Grossberg 1987](https://doi.org/10.1016/S0734-189X(87)80014-2)) — **secundária**: sem categoria que passe pelo limiar de vigilância ρ, uma nova é criada com a própria entrada.

## O que o Child tira disto

- **Dendrito:** um segmento com ativação em janela (dispara entre dois limiares) reproduz o XOR de Gidon 2020; o platô dura dezenas de ms (Schiller 2000).
- **Cerebelo:** grânulos com 4 entradas, poucos ativos (expansão esparsa), e leitura por regra delta guiada pelo erro (fibra trepadeira).
- **Hipocampo:** memória rápida e esparsa de episódios, repetida de forma intercalada durante o `sleep`.
- **Neurogênese:** criar neurônios quando a surpresa fica alta e a capacidade se esgota (Draelos 2017, Fritzke 1995), com teto configurável e neurônios novos mais plásticos (Ge 2007).
