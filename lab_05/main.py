# Неклепаева Анастасия, ИУ7-73Б
# Доп. задание: доставка сообщений выполняется с регулярным интервалом. 
# Интервал и тело сообщения, имя файла для прикрепления (опционально) 
# вводятся с клавиатуры. 

import smtplib
import sys
import os
import time
from email.mime.application import MIMEApplication
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email.utils import COMMASPACE, formatdate
from email import encoders

host = "smtp.gmail.com"
port = 465

def main():
    try:
        # адрес получателя
        receiver_address = sys.argv[1] 
        # адрес отправителя
        sender_address = sys.argv[2] 
        # пароль отправителя
        password = sys.argv[3]
    except Exception:
        print("Usage: receiver_address sender_address password") 
        return -1   
    
    try:
        # введение интервала времени отправки
        interval = int(input("Введите интервал времени доставки:"))
        # введение имени файла для прикрепления 
        filename = input("Введите имя файла для прикрепления:")
        # введение текста сообщения
        message_body = input("Введите сообщение:")
    except Exception:
        print("Error with input info") 
        return -1 
    
    os.chdir('/home/anastasia/Computer_networks/lab_05/files')

    # получение файлов директории
    files = os.listdir()
      
    # формирование сообщения из нескольких частей
    msg = MIMEMultipart()
    # задание отправителя
    msg['From'] = sender_address
    # задание получателя
    msg['To'] = receiver_address
    # задание формата времени
    msg['Date'] = formatdate(localtime=True)
    # задание темы сообщения
    msg['Subject'] = 'Computer networks: lab_05'
    # присоединение к письму сформированного тела в текстовом формате
    msg.attach(MIMEText(message_body, 'plain'))  
    
    # если файл не найден, то сообщение отправляется без файла
    if(files.count(filename) == 0):
        print('File not found')
    # иначе прикрепляется файл
    else:
        # формирование тела для вложения
        part = MIMEBase('application', "octet-stream")
        # загрузка в тело письма файла 
        part.set_payload(open(filename, "rb").read())
        # кодировка письма в base64
        encoders.encode_base64(part)
        # добавление заголовока к Content-Disposition контентной части письма
        # указание имени файла
        part.add_header('Content-Disposition', 'attachment; filename="{}"'.format(filename))
       
        # присоединение к письму сформированного тела с вложением
        msg.attach(part)
        
    try:
        # создание SMTP сессии по защищенному протоколу с сервером отправки
        server_ssl = smtplib.SMTP_SSL(host, port)
        # авторизация на сервере
        server_ssl.login(sender_address, password)
        
        # пока клиент не завершится по Ctrl+C, отправка сообщения 
        # с заданным интервалом
        while True:
            time.sleep(interval)
            server_ssl.sendmail(sender_address, receiver_address, msg.as_string())
            print ('Messages successful send!')
            
    # по Ctrl+C закрыть SMTP сессию
    except KeyboardInterrupt:
        server_ssl.close()
        print("Server closed")       
    except Exception:
        print("ERROR")
        
    return 0
        
if __name__ == "__main__":
    main()
