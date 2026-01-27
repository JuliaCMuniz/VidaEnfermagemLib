# Blueprint BP_Bubble_StoryTellerBase

Contém o Blueprint do StoryTeller, responsável por coordenar o avanço das histórias dentro da cena.

## Class Defaults

A propriedade Visible Bubbles pode ser alterada para aumentar ou diminuir o número de esferas exibidas para as próximas histórias. Por padrão, o máximo é 3. Isso significa que marcadores referentes a até as próximas 3 histórias ficarão visíveis a cada término de história. 

## EventGraph

### BeginPlay
Na inicialização da classe, verifica-se a quantidade de histórias na variável StoryList. Se houver histórias, inicia as rotinas pré-história SpawnBubbles (função) e BubblePrep (evento). 

### BubblePrep
Após a execução do SpawnBubbles, é verificado se há bolhas instanciadas. Se houver, associa o OnBubbleDestroyed da primeira bolha ao evento BubblePop e chama a sua função SetBubbleRight.

Como a primeira bolha corresponde à história em execução e todas as bolhas por padrão iniciam como "not right", apenas a primeira bolha passa a ter um feedback positivo. Além disso, a distruição dela, com a execução da rotina "right", continua o procedimento de execução da história. 

Se não houver bolhas na lista após o SpawnBubbles significa que a história atual não possui um marcador de âncora válido e deve ser executada imediatamente. 

### BubblePop
Após a bolha correta ser destruída (com a interação do usuário), todas as bolhas existentes na cena são removidas e a rotina StartStory é chamada.

### StoryFinished
A mensagem de finalização da história inicia a execução do evento StoryFinished que:
1. Ou envia uma mensagem OnStoryTellerFinished após todas as histórias serem concluídas;
2. Ou avança a execução para a próxima história. 
O index marcador da história é incrementado e as mesmas rotinas pré-história do início são chamadas novamente: SpawnBubbles e BubblePrep.

## Funções

### StartStory
Rotina executada para inicializar uma história. Se o objeto de CurrentStory for válido, isso significa que a história anterior não foi destruída corretamente, então ela deve ser removida. Do contrário, a próxima história pode ser instanciada. Sua referência é guardada na variável CurrentStory, o dispatcher OnStoryFinished da história é associado ao evento StoryFinished e é chamada a sua rotina de inicialização.

### SpawnBubbles
Rotina para instanciar os marcadores visuais (bolhas) referentes às próximas histórias disponíveis para o usuário. 

Enquanto houver marcadores válidos de âncoras dentro da quantidade limite de bolhas visíveis estabelecido pela variável VisibleBubbles, instancia bolhas na posição das âncoras correspondentes, insere suas referências na lista de bolhas ativas, altera o valor da sua propriedade BubbleIndex para o seu index na lista e associa cada dispatcher GetErrorMessage à função SetErrorMessage.

### CleanBubbleArray
Rotina para destruir todos os objetos válidos da lista de bolhas ativas.

### SetErrorMessage
A partir do index da bolha ativa, verifica se há uma mensagem de erro correspondente no array de mensagens da história ativa. Se houver, busca o widget de erro na cena e troca a sua mensagem para o valor desse array.

O array de mensagens de erro de uma história precisa ter n-1 mensagens, onde n é o número máximo de bolhas visíveis. Isso, pois as mensagens são exibidas caso a próxima n-ésima história seja selecionada ao invés da correta. Nesse sentido, sendo correta a história 0, a mensagem de erro caso a história 1 seja selecionada será a mensagem 0 da lista de mensagens da história 0. Assim, também foi adicionado um offset de -1 na busca da mensagem dentro do array.

