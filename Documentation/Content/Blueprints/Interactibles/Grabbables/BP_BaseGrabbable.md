# Blueprint BP_BaseGrabbable

Classe base para objetos que podem ser segurados pelo jogador utilizando gestos de grab reconhecidos pelo plugin Interactions SDK da Meta.

## Componentes

O módulo possui uma mesh, um componente Isdk Grab Interactible e um componente IsdkGrabbable.

## EventGraph

Para reutilização da lógica, partiu-se de uma mesh genérica com o valor a ser atribuído nas subclasses. Dessa forma, foi necessária a implementação da lógica de forma dinâmica, a partir do EventBeginPlay:

1. Associar a zona de colisão da mesh ao componente Isdk Grab Interactible para definir os triggers de interação. 
2. Associar o grabbable ao grab interactible para que as funções do grabbable possam ser incorporadas de forma dinâmica. 
3. Construir um transformer para definição do tipo de movimento do objeto. No caso do projeto foi escolhido o FreeTransformer, mas é possível escolher e combinar outros tipos de transformer para adicionar restrições no movimento. 
4. Associar esse transformer ao grabbable. 

Assim, com interações de gestos reconhecidas na colisão da mesh, o grabbable poderá aplicar transformações à ela. 