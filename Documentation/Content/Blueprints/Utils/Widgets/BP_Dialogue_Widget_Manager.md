# Blueprint BP_Dialogue_Widget_Manager

Blueprint que herda do ISDK Interactable Widget da Meta, responsável por converter interações 2D com widgets para interações reconhecidas pelo sistema de rastreamento de mãos do plugin. Utilizado para conter o widget de diálogo.

## Detalhes de Operação

Foi notado que caso o widget seja iniciado com a visibilidade em *false*, não ocorre a renderização do widget. Além disso, a atribuição dinâmica do widget, por exemplo no Event BeginPlay coletando o valor de alguma variável, resulta em uma não renderização do widget. As funções responsáveis pela renderização ocorrem em um nível a priori inacessível ao desenvolvedor e, durante o desenvolvimento do projeto, não se encontrou forma de tratamento direto dessas situações.

A abordagem escolhida foi a definição de classes que herdam do widget contendo por padrão no Interactable Widget Component (Root) o widget a ser exibido. Além disso, houve também a adição de propriedades para parametrizar o uso desse widget em cena. Dessa forma, um gerenciador instancia essa classe com o widget padronizado e aplica as operações necessárias a partir das propriedades definidas.

## Variáveis

1. DialogueLocation: Contém a posição em que o widget vai ser exibido;
2. DialogueRotation: Contém a rotação em que o widget vai ser exibido;
3. DialogueContent: Contém as perguntas e respostas do diálogo a ser exibido, definidos no widget do StoryBuilder.

