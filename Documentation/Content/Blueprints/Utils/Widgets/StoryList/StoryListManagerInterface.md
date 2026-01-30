# Interface StoryListManagerInterface

Interface que contém os métodos chamados pelos itens da lista de passos de uma história para efetivar alterações em um item da lista. Contém as funções:

1. GetAnchorsNamesMapKeys: para receber todas as tag de âncoras registradas, exibidas no dropdown da entrada;
2. GetStorysClassesMapKeys: para receber as strings correspondentes às possíveis classes de história, utilizadas no dropdown da entrada;
3. OnStoryDeleted: para remoção da história;
4. OnStoryChanged: para mudança em qualquer característica da história;
5. GetStorySteps: para buscar todos os passos da história;
6. AddStoryStep: para adicionar um passo vazio à história;
7. OpenMessageEditorForStory: para abrir o editor de mensagens de erro da história.