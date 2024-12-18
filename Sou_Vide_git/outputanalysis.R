library(readr)
library(tidyverse)
library(ggplot2)
temphist <- read_csv("Sou_Vide_git/temphist.csv", 
                     col_names = FALSE)
names(temphist) <- c("timestamp","var","value")
op <- options(digits.sec=3) 
t <-  temphist %>%
  mutate(timestamp1=substr(timestamp,1,13),
         timestamp1=strptime(timestamp,format="%H:%M:%OS"))%>% 
  filter(!is.na(timestamp)&!is.na(var) & var !="HeaterStatus") %>% 
  mutate(
         cycle1=ifelse(var=="Setpoint",1,0),
         cycle1=cumsum(cycle1)
         ) 
cycles <- t %>% 
  group_by(cycle1) %>% 
  summarise(cycle_start=min(timestamp1)) %>% 
  filter(cycle1>0)

t <- left_join(cycles,t)
t <- t %>% select(-timestamp1,-timestamp,-cycle1)

data <- t %>% 
  pivot_wider(names_from = "var",values_from = "value") %>% 
  mutate(delay = as.numeric(difftime(cycle_start,lag(cycle_start),units = "secs")))
data$delay[1]=10
data <- data %>% 
  mutate(timeoffset=ifelse(delay<0,24*3600,0),
         timeoffset=cumsum(timeoffset),
         cycle_start=cycle_start+timeoffset
         )


data %>% ggplot(aes(x=cycle_start,y=Error)) +
  geom_point()
