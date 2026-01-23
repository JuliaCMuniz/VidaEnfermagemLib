# Pasta SoapDispenser

Contém o Blueprint SoapDispenser, reconstruído a partir do blueprint original e implementado no projeto.

## Blueprint SoapDispenser

### Componentes

O módulo contém uma mesh, um campo de colisão, e os sistemas de som e partículas construídos para a saboneteira (presentes na pasta Effects). 

### EventGraph

A operação da saboneteira é baseada em colisões com o campo definido. A partir de colisões com o jogador, as rotinas de exibição de partícula e som são iniciadas. Enquanto isso, colisões com mãos específicas iniciam rotinas para validar o ensaboamento de cada mão. 

A identificação da mão como direita ou esquerda é feita a partir de tags nos componentes MotionController do VRPawnCustom (classe construída para o jogador).

### Funções

1. SoapLeftHand / SoapRightHand: Altera o valor da variável LeftHandSoaped / RightHandSoaped para *true* e chama o dispatcher de OnAnyHandSoaped. Depois, verifica se a outra variável (RightHandSoaped / LeftHandSoaped) também apresenta o valor *true*, se sim, chama o dispatcher OnBothHandsSoaped.

2. ShowSoap: Aciona o sistema de som, com fade in, e o sistema de partículas.

3. EndSoap: Desativa o sistema de som, com fade out, e o sistema de partículas.
