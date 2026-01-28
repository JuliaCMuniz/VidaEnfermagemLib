# Blueprint BP_Action_Bubble

Blueprint da classe de esferas usadas como marcadores visuais para os passos e as histórias da cena.

## Default Values

No menu de Default Values é possível escolher o material das esferas no estado padrão, quando correta e quando incorreta.

## EventGraph

### BeginPlay

Associa o dispatcher OnAnyHandTrigger ao evento SetBubbleWarning. Esse é o comportamento padrão de todas as esferas e indica que a esfera selecionada é incorreta.

### ActorBeginOverlap

*** substituir pelo método de detecção de componente ***

Contém a rotina para detectar que o VRPawn entrou em contato com a esfera do marcador, especificamente qual das mãos foi utilizada, e inicializa dispatchers correspondetes (OnLeftHandTrigger, OnRightHandTrigger, OnAnyHandTrigger).

### DestroyBubble

Evento responsável por destruir o ator da classe após a esfera correta ser selecionada. Chama o dispatcher OnBubbleDestroyed antes de remover o ator.

### SetBubbleSuccess

Rotina responsável por indicar que a esfera selecionada é a correta, troca a cor da esfera pelo material definido nos Default Values e após 300 ms aciona o evento DestroyBubble.

### SetBubbleWarning

Rotina responsável por indicar que a esfera selecionada é incorreta, troca a cor da esfera para a cor definida nos Default Values e chama o dispatcher GetErrorMessage. 

### SetBubbleDefault

Após a finalização da rotina de exibição da mensagem de erro e a interação do usuário com o botão de fechar do widget da mensagem, retorna a esfera com o indicativo de incorreto para o estado padrão.

## Funções

### SetBubbleRight

Troca o bind do OnAnyHandTrigger do padrão de SetBubbleWarning para SetBubbleSuccess.