# Widget AnchorTagListEntryWidget

Widget de um elemento da lista de tags de âncoras, contém um campo de texto editável e um botão de remoção do elemento.

## Graph

### EventGraph

#### OnListItemObjectSet

- Esse evento é chamado na criação do item da lista, através da passagem de um objeto com o formato AnchorTagListEntryObject dentro do editor widget StoryBuilder. 
    
Configura a variável AnchorTagListEntryObject do widget para referenciar o objeto de entrada e atribui o valor padrão do campo de texto como o valor de tag recebido. 

#### OnClicked (RemoveButton)

Após o botão de remoção ser clicado, encontra o widget com a interface AnchorTagListManagerInterface e chama o método OnTagDeleted, passando o valor de index recebido do objeto de entrada. 

#### OnTextCommited (Editable Text)

Após o campo de texto editável perder o foco, atualiza o objeto referenciado pela variável AnchorTagListEntryObject para conter o novo valor de texto, encontra o widget com a interface AnchorTagListManagerInterface e chama o método OnTagChanged, passando o index e o novo valor de tag.

#### OnTextChanged (Editable Text)

A cada alteração realizada no texto, atualiza o valor do objeto referenciado pela variável AnchorTagListEntryObject para conter o novo valor, mantendo o campo de texto editável coerente com a variável.

### Funções

#### Get_EditableText_Text

Função que define como o campo de texto editável é populado, associando seu valor sempre ao campo de texto da variável AnchorTagListEntryObject. Dessa forma, o campo de texto pode ser atualizado de forma dinâmica, seguindo a variável.

Essa associação é definida dentro do menu Designer, com a opção de bind do conteúdo do campo de texto.

#### UpdateAnchorVar

Constrói um novo objeto da classe AnchorTagListEntryObject e substitui a referência da variável com o mesmo nome.

