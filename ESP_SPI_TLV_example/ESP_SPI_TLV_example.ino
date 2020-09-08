/* Basic Type Length Value Communication over SPI 
 */
#include <SPI.h>

static const int spiClk = 1000000; // 1 MHz

uint8_t _type     = 0x28;
uint8_t _length   = 0x02;
uint8_t _value [] = {0x0D,0x01};

uint8_t _SPIread     = 0x00;

uint8_t TLV_DUMMY = 0xFF; 

int errorInSPI ; 
//uninitalised pointers to SPI objects
SPIClass * hspi = NULL;

void setup() {
  Serial.begin(115200) ; 
  Serial.println("started");
  //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
  hspi = new SPIClass(HSPI);
    
  //initialise hspi with default pins
  //SCLK = 14, MISO = 12, MOSI = 13, SS = 15
  hspi->begin(); 

  //set up slave select pins as outputs as the Arduino API
  //doesn't handle automatically pulling SS low
  pinMode(15, OUTPUT); //HSPI SS
  delay (1000) ; 
}

// the loop function runs over and over again until power down or reset
void loop() {
  //use the hSPI 
  //hspiTLVsend();
  // Communication Flow:
 
// ============== Send a TLV request ================   
  Serial.println("Sending TLV");  
  
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));  
  digitalWrite(15, LOW);
  
  // Send type
  _SPIread = hspi->transfer(_type);
  // sync and check for errors
  errorInSPI = 0 ; 
  // Should receive 0xFF (dummy data)
  Serial.println(_SPIread, HEX);   
  if (_SPIread != TLV_DUMMY) {
    errorInSPI += 1  ; 
  } 
  
  // Send length
  _SPIread = hspi->transfer(_length);
  
  // Should receive 0xFF (dummy data)
  Serial.println(_SPIread, HEX);   
  if (_SPIread != TLV_DUMMY) {
    errorInSPI += 1  ; 
  } 
  
  // Send values   
  for(int i=0; i < sizeof (_value) ; i++) {
    _SPIread = hspi->transfer(_value[i]);
    // Should receive 0xFF (dummy data)
    Serial.println(_SPIread, HEX);   
    if (_SPIread != TLV_DUMMY) {
      errorInSPI += 1  ; 
    } 
  }  

  // End transfer
  digitalWrite(15, HIGH);
  hspi->endTransaction();  

  if (errorInSPI) {
    Serial.println("Error in TLV send");
  }
// ============== delay at least 1ms ================     
  delay(1);
  if (!errorInSPI) {
    // ============== Read TLV header ================      
      Serial.println("Receiving TLV");
      hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));  
      digitalWrite(15, LOW);
         
      // ============== Clock in SIZE, clock out don't care ================  
      uint8_t _size = hspi->transfer(0);      
      Serial.println(_size, HEX);  
      
      // ============== Clock in NUM, clock out don't care ================
      // NUM is always of value 1 except for API function dwm_backhaul_xfer
      uint8_t _num = hspi->transfer(0);  
      Serial.println(_num, HEX);  
    
      // End transfer
      digitalWrite(15, HIGH);
      hspi->endTransaction();  
      
      // ==============  delay ================     
      delay(1);
      // ============== read the TLV response ================    
      Serial.println("Receiving payload");
      
      hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));  
      digitalWrite(15, LOW);  
      
      if (_size > 0 && _num > 0 && _size!=TLV_DUMMY && _num!=TLV_DUMMY ) {
        for(int i=0; i < _size ; i++) {
          _SPIread = hspi->transfer(0);  
          Serial.println(_SPIread, HEX); 
        }  
      } else {
        Serial.println("The response is not ready");
      }   
      
      digitalWrite(15, HIGH);
      hspi->endTransaction();  
  } 
  
  delay(1);
  
}
