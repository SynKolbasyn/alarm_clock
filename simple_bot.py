from aiogram import Bot, Dispatcher, types, Router, F
from aiogram.filters import Command, StateFilter
from aiogram.types import Message
from aiogram.types import FSInputFile, URLInputFile, BufferedInputFile
from model import *

import os

import asyncio
from aiogram.fsm.context import FSMContext


router = Router()
bot = Bot(token='6159335819:AAEzWZWz_Vao4fmzFxr_7s2MdOzsQyS6z6w')

dp = Dispatcher()





async def download(message: types.Message, bot: Bot):
    await bot.download(
        message.photo[-1],
        destination=f"./{message.photo[-1].file_id}.jpg"
    )




@dp.message(F.photo)
async def download_photo(message: Message, bot: Bot):
    task = asyncio.create_task(download(message, bot))
    done, pending1 = await asyncio.wait([task], return_when=asyncio.FIRST_COMPLETED)
    path = f'{message.photo[-1].file_id}.jpg'
    pic_to_skeleton_tg(path)

    image_from_pc = FSInputFile(f"/home/n0mad/PycharmProjects/cursed work/new_{message.photo[-1].file_id}.jpg")
    result = await message.answer_photo(
        image_from_pc,
        caption="Правильно ли мы обработали позу? Чтобы обонвить позу, пришлите картинку еще раз."
    )

@dp.message(Command('start'))
async def start(message: types.Message):
    await message.answer("велкам! отправь картинку, а я нарисую твою позу.")

# Запуск процесса поллинга новых апдейтов
async def main():
    await dp.start_polling(bot)

if __name__ == '__main__':
    asyncio.run(main())