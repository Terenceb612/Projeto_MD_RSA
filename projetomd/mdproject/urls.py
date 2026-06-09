from django.urls import path
from . import views
app_name='mdproject'
urlpatterns = [
    path('', views.index, name='index'),
    path('upload-c-zip/', views.upload_c_zip, name='upload_c_zip'),
    path('save/', views.save, name='save'),
    path('crypt/', views.crypt, name='crypt'),
    path('descrypt/', views.descrypt, name='descrypt'),
    path('reset/', views.reset, name='reset'),
    path('decrypt-external/', views.decrypt_external, name='decrypt_external'),
]
