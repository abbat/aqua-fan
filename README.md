# aqua-fan

Проект кулера (вентиляторного охлаждения) для аквариума с крышкой. Позволяет понизить температуру воды в аквариуме на 4°C (±0.3°C) относительно комнатной, что актуально для помещений без кондиционера в летний период.

Изначально разрабатывался для аквариума [EHEIM vivalineLED 126](https://eheim.com/en_GB/aquatics/aquariums/aquariums-fresh-water/vivalineled/vivalineled-126), но это ограничение касается только модели корпуса для печати на 3D принтере (в программно-аппаратную же часть заложено масштабирование на различные конфигурации).

<p align='center'>
<img src='https://user-images.githubusercontent.com/802583/176910915-12d54ec3-abb3-45e9-9a75-8bc50e7af132.jpg' alt='EHEIM vivaline aquarium fan cooler' title='Вентиляторный кулер для аквариума EHEIM vivaline 126'>
<img src='https://user-images.githubusercontent.com/802583/176910916-9e04b4f2-727f-4699-af2c-489124b665e2.jpg' alt='EHEIM vivaline aquarium fan cooler' title='Вентиляторный кулер для аквариума EHEIM vivaline 126'>
</p>

## Возможности

* Крепление на короб (крышку). Все коммерческие решения (например [GHL Aquarium Fans](https://www.aquariumcomputer.com/products/ghl-cooling-technology/aquarium-fans-propellerbreeze/), [Aqua Medic Arctic Breeze](https://www.aqua-medic.de/index.php?r=catalog/product&id=66&cid=34)) предполагают крепление блока вентиляторов клипсами на край стекла толщиной до 21 мм. Однако ширина короба аквариума данного типа 26 мм., что делает невозможным крепление на клипсы и потребовало бы открытия крышки (если бы крепление все же было возможно).
* Низкий уровень шума благодаря автоматической регулировке скорости вращения вентиляторов в зависимости от разницы температур (в отличии, например, от контроллеров [Aqua Medic Cool Control](https://www.aqua-medic.de/index.php?r=catalog/product&id=475&cid=34), у которых вентиляторы всегда работают на максимально разрешенной скорости).
* Контроль и подстройка скорости вращения по показаниям тахометров самих вентиляторов. Все коммерческие решения используют регулировку скорости вращения через изменение напряжения питания без использования обратной связи, а для различных моделей вентиляторов зависимость скорости вращения от напряжения может быть нелинейной (например см. спецификацию [be quiet! Pure Wings 2](https://www.bequiet.com/ru/casefans/505)) и разной даже для одной модели вентиляторов в блоке.

<p align='center'>
<img src='https://user-images.githubusercontent.com/802583/176910921-0149d1dd-c7f8-4fe7-a481-56580e42d055.jpg' alt='EHEIM vivaline aquarium fan cooler' title='EHEIM vivaline aquarium fan cooler'>
<img src='https://user-images.githubusercontent.com/802583/176910923-16f9297a-8f9c-4ea3-a6b2-05dd4e00aaa7.jpg' alt='EHEIM vivaline aquarium fan cooler' title='EHEIM vivaline aquarium fan cooler'>
</p>

## Материалы

* [3D-модели](3d-model.md)
* [Аппаратное обеспечение](hardware.md)
* [Электрическая схема](schema.md)
* [Программное обеспечение](software.md)
* [Поддержка WiFi](wifi.md)

<p align='center'>
<img src='https://user-images.githubusercontent.com/802583/176910924-86dc4556-deb0-4e95-aaa6-3afc42466d41.jpg' alt='EHEIM vivaline aquarium fan cooler'  title='Вентиляторный кулер для аквариума с крышкой'>
<img src='https://user-images.githubusercontent.com/802583/176910927-9f6b0f0d-b652-459e-a28e-c45b7cc96132.jpg' alt='EHEIM vivaline aquarium fan cooler'  title='Вентиляторный кулер для аквариума с крышкой'>
</p>

## Как можно помочь?

* Переведите этот документ или связанные материалы на свой родной язык;
* Исправляйте ошибки в этом документе или связанных материалах для своего родного языка;
* Делитесь информацией со своими друзьями;
* Отправляйте PR если вы имеете нестандартный аквариум под который не подходят коммерческие решения, разработчик, схемотехник или успешны в 3D моделировании и печати.
