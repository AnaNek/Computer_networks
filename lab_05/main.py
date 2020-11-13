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
        receiver_address = sys.argv[1] 
        sender_address = sys.argv[2] 
        password = sys.argv[3]
    except Exception:
        print("Usage: receiver_address sender_address password") 
        return -1   
    
    try:
        interval = int(input("Введите интервал времени доставки:"))
        filename = input("Введите имя файла для прикрепления:")
        message_body = input("Введите сообщение:")
    except Exception:
        print("Error with input info") 
        return -1 
    
    os.chdir('/home/anastasia/Computer_networks/lab_05/files')

    files = os.listdir()
      
    msg = MIMEMultipart()
    msg['From'] = sender_address
    msg['To'] = receiver_address
    msg['Date'] = formatdate(localtime=True)
    msg['Subject'] = 'Computer networks: lab_05'
    msg.attach(MIMEText(message_body, 'plain'))  
    
    if(files.count(filename) == 0):
        print('File not found')
    else:
        part = MIMEBase('application', "octet-stream")
        part.set_payload(open(filename, "rb").read())
        encoders.encode_base64(part)
        part.add_header('Content-Disposition', 'attachment; filename="{}"'.format(filename))
       
        msg.attach(part)
        
    try:
        server_ssl = smtplib.SMTP_SSL(host, port)
        server_ssl.login(sender_address, password)
        time.sleep(interval)
        server_ssl.sendmail(sender_address, receiver_address, msg.as_string())
            
        server_ssl.close()
            
        print ('Messages successful send!')
    except Exception:
        print("ERROR")
    return 0
        
if __name__ == "__main__":
    main()
