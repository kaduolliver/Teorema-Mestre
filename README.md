# Aplicação do Teorema Mestre

Trabalho da disciplina **Análise de Algoritmos**, da **Universidade Católica de Brasília**.

O projeto apresenta três algoritmos recursivos em C, suas recorrências, demonstrações por indução matemática e comparação entre as complexidades teóricas e os resultados experimentais.

## Algoritmos analisados

| Algoritmo                           | Recorrência                | Caso | Complexidade       |
| ----------------------------------- | -------------------------- | ---- | ------------------ |
| Máximo por divisão em quatro partes | \(T(n)=4T(n/4)+\Theta(1)\) | 1    | \(\Theta(n)\)      |
| Busca binária recursiva             | \(T(n)=T(n/2)+\Theta(1)\)  | 2    | \(\Theta(\log n)\) |
| Soma com recursão e varredura       | \(T(n)=T(n/2)+\Theta(n)\)  | 3    | \(\Theta(n)\)      |

## Arquivos

* `teorema_mestre.c`: implementações, medição de tempo e contagem de operações.
* `resultados_experimentos.csv`: dados da campanha experimental.

## Compilação e execução

Requer **GCC e ambiente Linux/POSIX**.

```bash
# Medição de tempo
gcc -std=c11 -O2 -Wall -Wextra -fno-optimize-sibling-calls teorema_mestre.c -o tempo

# Contagem de operações
gcc -std=c11 -O2 -Wall -Wextra -fno-optimize-sibling-calls -DINSTRUMENT teorema_mestre.c -o contagem

# Exemplo: Problema 1, n = 4⁴ = 256, sete amostras
./tempo 1 4 7 nova_execucao.csv
./contagem 1 4 1 nova_execucao.csv
```

Argumentos: **problema**, **expoente**, **número de amostras** e **arquivo CSV**. O tamanho é `n = 4^k` no Problema 1 e `n = 2^k` nos Problemas 2 e 3. Os registros são acrescentados ao arquivo informado.

## Autores

* Carlos Eduardo Lima de Oliveira
* Henrique Kümmel do Rosário
