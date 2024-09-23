
import datetime  

def daily_commit(start_date, end_date):  
    # Sample implementation of what the function might do  
    current_date = start_date  
    while current_date <= end_date:  
        print(f"Committing for date: {current_date}")  
        current_date += datetime.timedelta(days=1)  
        if __name__ == '__main__':
              daily_commit(datetime.date(2024, 6, 1), datetime.date(2024,7, 1))