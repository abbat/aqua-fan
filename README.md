# aqua-fan

Проект кулера (вентиляторного охлаждения) для аквариума с крышкой. Изначально разрабатывался для аквариума [EHEIM vivalineLED 126](https://eheim.com/en_GB/aquatics/aquariums/aquariums-fresh-water/vivalineled/vivalineled-126), но это ограничение касается только модели корпуса для печати на 3D принтере (в программно-аппаратную же часть заложено масштабирование на различные конфигурации).

<p align='center'>
<img src='https://user-images.githubusercontent.com/802583/176043032-9bc03607-fb3f-487b-b138-808c0c32084e.jpg' alt='EHEIM vivaline aquarium fan cooler'>
<img src='https://user-images.githubusercontent.com/802583/176043040-47506f47-3a48-41ff-99fd-839eee1d8771.jpg' alt='EHEIM vivaline aquarium fan cooler'>
</p>

## возможности

* Крепление на короб (крышку). Все коммерческие решения (например, [GHL Aquarium Fans](https://www.aquariumcomputer.com/products/ghl-cooling-technology/aquarium-fans-propellerbreeze/), [Aqua Medic Arctic Breeze](https://www.aqua-medic.de/index.php?r=catalog/product&id=66&cid=34)) предполагают крепление блока вентиляторов клипсами на край стекла толщиной до 21 мм. Однако ширина короба аквариума данного типа 26 мм., что делает невозможным крепление на клипсы и потребовало бы открытия крышки (если бы крепление все же было возможно).
* Низкий уровень шума благодаря автоматической регулировке скорости вращения вентиляторов в зависимости от разницы температур (в отличии, например, от контроллеров [Aqua Medic Cool Control](https://www.aqua-medic.de/index.php?r=catalog/product&id=475&cid=34), у которых вентиляторы всегда работают на максимально разрешенной скорости).
* Контроль и подстройка скорости вращения по показаниям тахометров самих вентиляторов. Все коммерческие решения используют регулировку скорости вращения через изменение напряжения питания без использования обратной связи, а для различных моделей вентиляторов зависимость скорости вращения от напряжения может быть нелинейной (например, см. спецификацию [be quiet! Pure Wings 2](https://www.bequiet.com/ru/casefans/505)) и разной даже для одной модели вентиляторов в блоке.
