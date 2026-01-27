# Blueprint BP_Bubble_StoryBase

Contém o Blueprint de uma Story, responsável por coordenar o avanço de Steps dentro de histórias na cena.

## Class Defaults

A propriedade Visible Bubbles pode ser alterada para aumentar ou diminuir o número de esferas exibidas para os próximos passos. Por padrão, o máximo é 3. Isso significa que marcadores referentes a até os próximas 3 passos ficarão visíveis a cada término de passo. 

## EventGraph

### BeginPlay
Na inicialização da classe, verifica-se a quantidade de passos na variável StepList. Se houver passos, inicia as rotinas pré-passo SpawnBubbles (função) e BubblePrep (evento). 

### BubblePrep
Após a execução do SpawnBubbles, é verificado se há bolhas instanciadas. Se houver, associa o OnBubbleDestroyed da primeira bolha ao evento BubblePop e chama a sua função SetBubbleRight.

Como a primeira bolha corresponde ao passo em execução e todas as bolhas por padrão iniciam como "not right", apenas a primeira bolha passa a ter um feedback positivo. Além disso, a distruição dela, com a execução da rotina "right", continua o procedimento de execução do passo. 

Se não houver bolhas na lista após o SpawnBubbles significa que o passo atual não possui um marcador de âncora válido e deve ser executado imediatamente. 

### BubblePop
Após a bolha correta ser destruída (com a interação do usuário), todas as bolhas existentes na cena são removidas e a rotina StartStep é chamada.

### StepFinished
A mensagem de finalização do passo inicia a execução do evento StoryFinished que:
1. Ou envia uma mensagem OnStoryTellerFinished após todas os passos serem concluídos;
2. Ou avança a execução para a próxima história. 
O index marcador do passo é incrementado e as mesmas rotinas pré-história do início são chamadas novamente: SpawnBubbles e BubblePrep.

## Funções

### StartStep
Rotina executada para inicializar uma passo. Se o objeto de CurrentStep for válido, isso significa que o passo anterior não foi destruído corretamente, então ele deve ser removido. Do contrário, o próximo passo pode ser instanciado. Sua referência é guardada na variável CurrentStep e o dispatcher OnStepFinished do passo é associado ao evento StepFinished.

### SpawnBubbles
Rotina para instanciar os marcadores visuais (bolhas) referentes aos próximos passos disponíveis para o usuário. 

Enquanto houver marcadores válidos de âncoras dentro da quantidade limite de bolhas visíveis estabelecido pela variável VisibleBubbles, instancia bolhas na posição das âncoras correspondentes, insere suas referências na lista de bolhas ativas, altera o valor da sua propriedade BubbleIndex para o seu index na lista e associa cada dispatcher GetErrorMessage à função SetErrorMessage.

### CleanBubbleArray
Rotina para destruir todos os objetos válidos da lista de bolhas ativas.

### SetErrorMessage
A partir do index da bolha ativa, verifica se há uma mensagem de erro correspondente no array de mensagens do passo ativo. Se houver, busca o widget de erro na cena e troca a sua mensagem para o valor desse array.

O array de mensagens de erro de um passo precisa ter n-1 mensagens, onde n é o número máximo de bolhas visíveis. Isso, pois as mensagens são exibidas caso o próximo n-ésimo passo seja selecionado ao invés do correto. Nesse sentido, sendo correto o passo 0, a mensagem de erro caso o passo 1 seja selecionado será a mensagem 0 da lista de mensagens do passo 0. Assim, também foi adicionado um offset de -1 na busca da mensagem dentro do array.

