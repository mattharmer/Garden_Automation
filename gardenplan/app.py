import pygame, sys

# General Setup
pygame.init()
clock = pygame.time.Clock()

# Create Display Surface
screen = pygame.display.set_mode((1000,800))

block_test = pygame.Surface([100,200])
block_test.fill((0,0,250))

gardenbed = pygame.image.load('img/bed1.png')
gardenbed_rect = gardenbed.get_rect(topleft = [100,200])

while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sys.exit()

    screen.fill((250,250,250))
    screen.blit(block_test,(0,50))
    screen.blit(gardenbed,gardenbed_rect)
    gardenbed_rect.right += 1
    pygame.display.flip()
    clock.tick(60) # Defines speed of animation