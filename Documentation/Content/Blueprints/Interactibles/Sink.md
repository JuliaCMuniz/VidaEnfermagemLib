# Pasta Sink

Contém o Blueprint BP_Sink, recuperado do blueprint original e implementado no projeto.

## Blueprint BP_Sink

### Componentes

O módulo contém uma mesh, um campo de colisão, e os sistemas de som (presenta na pasta de Effects de RefAssets) e partículas construídos para a pia (presentes na pasta Effects). 

### EventGraph

A operação da pia é baseada em colisões com o campo definido. A partir de colisões com o jogador, as rotinas de exibição de partícula e som são iniciadas. Enquanto isso, colisões com mãos específicas iniciam rotinas para validar o ensaboamento de cada mão. 

A identificação da mão como direita ou esquerda é feita a partir de tags nos componentes MotionController do VRPawnCustom (classe construída para o jogador).

### Funções

1. WashLeftHand / WashRightHand: Altera o valor da variável LeftHandWashed / RightHandWashed para *true* e verifica se a outra variável (RightHandWashed / LeftHandWashed) também apresenta o valor *true*, se sim, chama o dispatcher OnBothHandsWashed.

2. ShowWater: Aciona o sistema de som, com fade in, e o sistema de partículas.

3. EndWater: Desativa o sistema de som, com fade out, e o sistema de partículas.
