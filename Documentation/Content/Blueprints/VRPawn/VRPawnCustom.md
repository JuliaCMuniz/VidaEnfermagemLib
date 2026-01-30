# Blueprint VRPawnCustom

Blueprint contendo o modelo de peão desenvolvido para permitir o rastreamento de mãos dentro do ambiente VR.

## Componentes

Para utilizar o tracking e modelo de mãos do SDK da Meta, é necessário adicionar dois pares de componentes do tipo:

* Motion Controller Component
    * ISDK Hand Rig (Left/Right)

Os nomes dos componentes do ISDK variam conforme a versão do plugin e podem ser confirmados no [projeto de exemplo disponível no github da Meta](https://github.com/oculus-samples/Unreal-InteractionSDK-Sample) da versão correspondente da Unreal.

Além disso, também é necessário um Camera Component para adicionar a visão do usuário.

## EventGraph

Após a verificação se o HMD está ativo, identifica o chão do ambiente de acordo com as configurações de boundary do óculos e configura a origem do rastreamento para usá-lo como referência. Esse método de rastreamento foi escolhido por ser indicado para cenas nas quais o usuário se desloca fisicamente para interagir com a simulação.

Com isso, parte para a configuração do input do usuário. Coletando apenas os inputs do usuário 0, uma vez que a cena é de um jogador, mapeia os inputs de mão.
