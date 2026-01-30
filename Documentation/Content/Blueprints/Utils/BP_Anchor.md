# Blueprint BP_Anchor

Contém a classe de âncora utilizada como marcador de posição. A âncora possui uma tag utilizada para identificá-la. Os métodos que utilizam âncoras buscam a primeira instância com uma determinada tag, então colocar a mesma tag em múltiplas âncoras pode gerar resultados variados.

## Funções

### SetAnchorTagValue

Função que recebe uma string e altera os valores da variável AnchorTag da âncora e da tag 0 da instância.